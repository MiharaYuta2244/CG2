#include "VisionCone.h"
#include "Collision.h"
#include "DirectXUtils.h"
#include "GameObjects/StageObjects/Wall/Wall.h"
#include "GameObjects/Stageobjects/Door/Door.h"
#include "GameObjects/Stageobjects/Glass/Glass.h"
#include "MathUtility.h"
#include <algorithm>
#include <cmath>

using namespace TinyEngine;
using namespace DirectX;

void VisionCone::Initialize(EngineContext* ctx, float radius, float angleDegrees, uint32_t segments) {
	ctx_ = ctx;

	// パラメータを保存しておく
	radius_ = radius;
	angleDegrees_ = angleDegrees;
	segments_ = segments;

	// トランスフォームの初期化
	transform_ = {
	    {1.0f, 1.0f,   1.0f},
        {0.0f, 0.0f,   0.0f},
        {0.0f, 0.005f, 0.0f}  // Zファイティング(床とのチラつき)防止
	};
	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// 視界用マテリアルの初期設定
	material_.color = {0.0f, 1.0f, 0.0f, 0.3f};
	material_.enableLighting = false;
	material_.enableFoging = false;
	material_.uvTransform = MathUtility::MakeIdentity4x4();
	material_.shininess = 1.0f;
	material_.envScale = 0.0f;

	// メッシュ（扇形ポリゴン）のビルド
	CreateMesh(radius, angleDegrees, segments);

	// 定数バッファの作成とマッピング
	CreateConstantBuffers();
}

void VisionCone::CreateMesh(float radius, float angleDegrees, uint32_t segments) {
	auto device = ctx_->object3dCommon->GetDxCommon()->GetDevice();

	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;

	// 頂点データの生成
	// 扇形の中心点
	VertexData centerVert;
	centerVert.position = {0.0f, 0.0f, 0.0f, 1.0f};
	centerVert.texcoord = {0.5f, 0.0f};     // UV空間の上部中央
	centerVert.normal = {0.0f, 1.0f, 0.0f}; // 真上向きの法線
	vertices.push_back(centerVert);

	// 円弧上の頂点群
	float angleRadian = MathUtility::DegreeToRadian(angleDegrees);
	float startAngle = -angleRadian / 2.0f;
	float angleStep = angleRadian / static_cast<float>(segments);

	for (uint32_t i = 0; i <= segments; ++i) {
		float currentAngle = startAngle + angleStep * static_cast<float>(i);

		// XZ平面上に広げる
		float x = radius * std::sin(currentAngle);
		float z = radius * std::cos(currentAngle);

		VertexData vert;
		vert.position = {x, 0.0f, z, 1.0f};

		// UV座標の計算
		float u = static_cast<float>(i) / static_cast<float>(segments);
		vert.texcoord = {u, 1.0f};

		vert.normal = {0.0f, 1.0f, 0.0f};
		vertices.push_back(vert);
	}
	vertexCount_ = static_cast<uint32_t>(vertices.size());

	// インデックスデータの生成
	for (uint32_t i = 0; i < segments; ++i) {
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i + 2);
	}
	indexCount_ = static_cast<uint32_t>(indices.size());

	// 頂点バッファの生成と転送
	UINT vertexBufferSize = sizeof(VertexData) * vertexCount_;
	vertexResource_ = DirectXUtils::CreateBufferResource(device, vertexBufferSize);

	VertexData* vertexMap = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexMap));
	std::copy(vertices.begin(), vertices.end(), vertexMap);
	vertexResource_->Unmap(0, nullptr);

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = vertexBufferSize;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// インデックスバッファの生成と転送
	UINT indexBufferSize = sizeof(uint32_t) * indexCount_;
	indexResource_ = DirectXUtils::CreateBufferResource(device, indexBufferSize);

	uint32_t* indexMap = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexMap));
	std::copy(indices.begin(), indices.end(), indexMap);
	indexResource_->Unmap(0, nullptr);

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = indexBufferSize;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void VisionCone::CreateConstantBuffers() {
	auto device = ctx_->object3dCommon->GetDxCommon()->GetDevice();

	// WVPバッファ
	wvpResource_ = DirectXUtils::CreateBufferResource(device, sizeof(TransformationMatrix));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));

	// Materialバッファ
	materialResource_ = DirectXUtils::CreateBufferResource(device, sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
}

void VisionCone::Update(const std::list<std::unique_ptr<Wall>>& walls, const std::list<std::unique_ptr<Door>>& doors, const std::list<std::unique_ptr<Glass>>& glasses) {
	// ワールド行列の再計算
	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// 頂点バッファをMapしてアクセス可能にする
	VertexData* vertices = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertices));

	vertices[0].position = {0.0f, 0.0f, 0.0f, 1.0f};

	float halfAngle = angleDegrees_ * 0.5f;
	float angleStep = angleDegrees_ / segments_;

	// 各セグメントについてレイキャストを行う
	for (uint32_t i = 0; i <= segments_; ++i) {
		float currentAngle = -halfAngle + (angleStep * i);
		float rad = currentAngle * (std::numbers::pi_v<float> / 180.0f);

		// ローカル座標系での最大到達点
		Vector3 localTarget = {std::sin(rad) * radius_, 0.0f, std::cos(rad) * radius_};

		// ワールド座標に変換してレイを生成
		Vector3 worldOrigin = transform_.translate;
		Vector3 worldTarget = MathUtility::Transform(localTarget, worldMatrix_);

		Segment ray;
		ray.origin = worldOrigin;
		ray.diff = {worldTarget.x - worldOrigin.x, worldTarget.y - worldOrigin.y, worldTarget.z - worldOrigin.z};

		float closestT = 1.0f;

		// 全ての壁に対して交差判定を行う
		for (const auto& wall : walls) {
			float t = 0.0f;
			if (Collision::Intersect(ray, wall->GetCollision(), t)) {
				// 最も手前にある壁の衝突地点を記録
				if (t < closestT) {
					closestT = t;
				}
			}
		}

		// 全てのドアに対して交差判定を行う
		for (const auto& door : doors) {
			if (!door->GetIsOpen()) {
				float t = 0.0f;
				if (Collision::Intersect(ray, door->GetCollision(), t)) {
					if (t < closestT) {
						closestT = t;
					}
				}
			}
		}

		// 全てのガラスに対して交差判定を行う
		for (const auto& glass : glasses) {
			float t = 0.0f;
			if (Collision::Intersect(ray, glass->GetCollision(), t)) {
				// 最も手前にある壁の衝突地点を記録
				if (t < closestT) {
					closestT = t;
				}
			}
		}

		// 衝突結果に基づいてローカルの頂点座標を更新
		vertices[i + 1].position = {localTarget.x * closestT, localTarget.y * closestT, localTarget.z * closestT, 1.0f};
		vertices[i + 1].texcoord.y = closestT;
	}

	// 頂点バッファのアンマップ
	vertexResource_->Unmap(0, nullptr);

	// WVP行列の計算
	Camera* camera = ctx_->object3dCommon->GetDefaultCamera();
	if (camera) {
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjectionMatrix_ = MathUtility::Multiply(worldMatrix_, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix_ = worldMatrix_;
	}

	// 定数バッファへ書き込み
	transformMatrixData_->WVP = worldViewProjectionMatrix_;
	transformMatrixData_->World = worldMatrix_;
	transformMatrixData_->WorldInverseTranspose = MathUtility::MakeIdentity4x4();

	*materialData_ = material_;
}

void VisionCone::Draw(const std::string& texturePath) {
	auto commandList = ctx_->object3dCommon->GetDxCommon()->GetCommandList();
	ctx_->object3dCommon->DrawSettingCommon(ctx_->textureManager);
	std::string path = texturePath.empty() ? "resources/textures/white.png" : texturePath; // 任意のデフォルト白テクスチャ
	commandList->SetGraphicsRootDescriptorTable(2, ctx_->textureManager->GetSrvHandleGPU(path));
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->DrawIndexedInstanced(indexCount_, 1, 0, 0, 0);
}