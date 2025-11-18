#include "particle.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "Model.h"
#include "Random.h"
#include "TextureManager.h"
#include "particleCommon.h"
#include <DirectXMath.h>
#include <fstream>
#include <sstream>

using namespace Microsoft::WRL;
using namespace DirectX;

void Particle::Initialize(ParticleCommon* particleCommon, TextureManager* textureManager, ModelManager* modelManager) {
	particleCommon_ = particleCommon;
	textureManager_ = textureManager;
	modelManager_ = modelManager;

	// SRVの作成（インスタンシング用）
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kNumMaxInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	// CPU/GPUハンドルをクラスメンバへ取得して保存
	instancingSrvHandleCPU_ = particleCommon_->GetDxCommon()->GetCPUDescriptorHandle(particleCommon_->GetDxCommon()->GetSrvDescriptorHeap(), particleCommon_->GetDxCommon()->GetDescriptorSizeSRV(), 3);
	instancingSrvHandleGPU_ = particleCommon_->GetDxCommon()->GetGPUDescriptorHandle(particleCommon_->GetDxCommon()->GetSrvDescriptorHeap(), particleCommon_->GetDxCommon()->GetDescriptorSizeSRV(), 3);

	// プリミティブモデルの作成
	modelData_ = CreatePrimitive();

	// インスタンシングデータ作成
	CreateInstancingResource();

	particleCommon_->GetDxCommon()->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU_);

	// 頂点データの初期化
	CreateVertexData();

	// マテリアルの初期化
	CreateMaterialData();

	// テクスチャ読み込み
	textureManager_->LoadTexture(modelData_.material.textureFilePath);

	// テクスチャ番号を取得して、メンバ変数に書き込む
	modelData_.material.textureIndex = textureManager_->GetSrvIndex(modelData_.material.textureFilePath);

	// カメラをセットする
	camera_ = particleCommon_->GetDefaultCamera();
}

void Particle::Update() {
#ifdef USE_IMGUI
	/*ImGui::Begin("Particle");

	if (ImGui::Button("Add Particle")) {
		particles_.push_back(MakeParticle());
		particles_.push_back(MakeParticle());
		particles_.push_back(MakeParticle());
	}

	ImGui::End();*/
#endif

	// ビルボードマトリックスの作成
	Matrix4x4 billboardmatrix = CreateBillboardMatrix();
	
	numInstance_ = 0;
	for (std::list<ParticleState>::iterator particleIterator = particles_.begin(); particleIterator != particles_.end();) {
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particles_.erase(particleIterator); // 生存期間が過ぎたParticleはlistから消す。戻り値が次のイテレータとなる
		}

		// 速度を設定
		(*particleIterator).transform.translate += (*particleIterator).velocity * kDeltaTime;
		(*particleIterator).currentTime += kDeltaTime;

		// ビルボード用
		Matrix4x4 scaleMatrix = MathUtility::MakeScaleMatrix((*particleIterator).transform.scale);
		Matrix4x4 translateMatrix = MathUtility::MakeTranslateMatrix((*particleIterator).transform.translate);
		worldMatrix_ =MathUtility::Multiply(MathUtility::Multiply(scaleMatrix, billboardmatrix),translateMatrix);

		if (camera_) {
			const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
			worldViewProjectionMatrix_ = MathUtility::Multiply(worldMatrix_, viewProjectionMatrix);
		} else {
			worldViewProjectionMatrix_ = worldMatrix_;
		}

		// 透明度の変更
		float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);

		if (numInstance_ < kNumMaxInstance) {
			instancingData_[numInstance_].WVP = worldViewProjectionMatrix_;
			instancingData_[numInstance_].World = worldMatrix_;
			instancingData_[numInstance_].color = (*particleIterator).color;
			instancingData_[numInstance_].color.w = alpha;

			// 生きているParticleの数を1カウントする
			++numInstance_;
		}

		// 次のイテレータに進める
		++particleIterator;
	}

	*materialData_ = material_;
}

void Particle::Draw() {
	if (numInstance_ <= 0) {
		return;
	}

	// 3Dオブジェクト描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	particleCommon_->DrawSettingCommon();

	auto commandList = particleCommon_->GetDxCommon()->GetCommandList();

	// wvp用のBufferの場所を設定
	commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU_);

	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定

	// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// SRVのDescriptorTableの先頭を設定。3はrootParameter[3]（Pixel用テクスチャ）である。
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(modelData_.material.textureFilePath));

	// 描画!(DrawCall/ドローコール)。
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), numInstance_, 0, 0);
}

ComPtr<ID3D12Resource> Particle::CreateBufferResource(ComPtr<ID3D12Device> device, size_t sizeBytes) {
	if (!device)
		return nullptr;

	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // uploadHeapを使う
	// 頂点にリソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeBytes; // リソースのサイズ。今回はVector4を3頂点分
	// バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

ModelData Particle::CreatePrimitive() {
	ModelData modelData;

	modelData.vertices.push_back({
	    .position = {-1.0f, 1.0f, 0.0f, 1.0f},
          .texcoord = {0.0f, 0.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 左上
	modelData.vertices.push_back({
	    .position = {1.0f, 1.0f, 0.0f, 1.0f},
          .texcoord = {1.0f, 0.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 右上
	modelData.vertices.push_back({
	    .position = {-1.0f, -1.0f, 0.0f, 1.0f},
          .texcoord = {0.0f, 1.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 左下
	modelData.vertices.push_back({
	    .position = {-1.0f, -1.0f, 0.0f, 1.0f},
          .texcoord = {0.0f, 1.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 左下
	modelData.vertices.push_back({
	    .position = {1.0f, 1.0f, 0.0f, 1.0f},
          .texcoord = {1.0f, 0.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 右上
	modelData.vertices.push_back({
	    .position = {1.0f, -1.0f, 0.0f, 1.0f},
          .texcoord = {1.0f, 1.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 右下

	modelData.material.textureFilePath = "resources/uvChecker.png";

	return modelData;
}

void Particle::CreateVertexData() {
	// 頂点リソースの作成
	vertexResource_ = CreateBufferResource(particleCommon_->GetDxCommon()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	vertexResource_->Unmap(0, nullptr);
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void Particle::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = CreateBufferResource(particleCommon_->GetDxCommon()->GetDevice(), sizeof(Material));
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialResource_->Unmap(0, nullptr);
	// 三角形の色
	material_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	material_.enableLighting = true;
	material_.enableFoging = false;
	// uvTransformなどのデータを設定
	material_.uvTransform = MathUtility::MakeIdentity4x4();
	// 反射強度
	material_.shininess = 1.0f;
	// materialDataに代入
	*materialData_ = material_;
}

void Particle::CreateInstancingResource() {
	// Instancing用のTransformationMatrixリソースを作る
	instancingResource_ = CreateBufferResource(particleCommon_->GetDxCommon()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance);

	// 書き込むためのアドレス取得
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	instancingResource_->Unmap(0, nullptr);

	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		instancingData_[index].WVP = MathUtility::MakeIdentity4x4();
		instancingData_[index].World = MathUtility::MakeIdentity4x4();
		instancingData_[index].color = {1.0f, 1.0f, 1.0f, 1.0f};
	}
}

ParticleState Particle::MakeParticle() { 
	ParticleState particle;
	particle.transform.scale = {1.0f, 1.0f, 1.0f};
	particle.transform.rotate = {0.0f, 0.0f, 0.0f};
	particle.transform.translate = {12.0f + RandomUtils::RangeFloat(-1, 1), 10.0f + RandomUtils::RangeFloat(-1, 1), RandomUtils::RangeFloat(-1, 1)};
	particle.velocity = {RandomUtils::RangeFloat(-1, 1), RandomUtils::RangeFloat(-1, 1), RandomUtils::RangeFloat(-1, 1)};
	particle.color = {RandomUtils::RangeFloat(0, 1), RandomUtils::RangeFloat(0, 1), RandomUtils::RangeFloat(0, 1), 1.0f};
	particle.lifeTime = RandomUtils::RangeFloat(50, 60);
	particle.currentTime = 0;
	return particle;
}

Matrix4x4 Particle::CreateBillboardMatrix() {
	Matrix4x4 backToFrontMatrix = MathUtility::MakeYawRotateMatrix(std::numbers::pi_v<float>);
	Matrix4x4 billboardmatrix;

	if (isBillboard_) {
		billboardmatrix = MathUtility::Multiply(backToFrontMatrix, camera_->GetWorldMatrix());
	} else {
		billboardmatrix = MathUtility::MakeIdentity4x4();
	}

	billboardmatrix.m[3][0] = 0.0f;
	billboardmatrix.m[3][1] = 0.0f;
	billboardmatrix.m[3][2] = 0.0f;

	return billboardmatrix;
}
