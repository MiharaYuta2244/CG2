#include "DirectXUtils.h"
#include "MathUtility.h"
#include "VisonCone.h"
#include <algorithm>
#include <cmath>

using namespace TinyEngine;
using namespace DirectX;

void VisionCone::Initialize(EngineContext* ctx, float radius, float angleDegrees, uint32_t segments) {
	ctx_ = ctx;

	// トランスフォームの初期化
	transform_ = {
	    {1.0f, 1.0f,   1.0f},
        {0.0f, 0.0f,   0.0f},
        {0.0f, 0.005f, 0.0f}  // Zファイティング(床とのチラつき)防止
	};
	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// 視界用マテリアルの初期設定
	material_.color = {0.0f, 1.0f, 0.0f, 0.3f}; // 半透明の緑色
	material_.enableLighting = false;           // 敵の視界は影に影響されず発光してほしいためライティングOFF
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

void VisionCone::Update() {
	// ワールド行列の再計算
	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

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