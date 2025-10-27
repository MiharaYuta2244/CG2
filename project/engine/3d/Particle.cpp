#include "MathOperator.h"
#include "MathUtility.h"
#include "Model.h"
#include "Particle.h"
#include "ParticleCommon.h"
#include "TextureManager.h"
#include <DirectXMath.h>
#include <fstream>
#include <sstream>

using namespace Microsoft::WRL;
using namespace DirectX;

void Particle::Initialize(ParticleCommon* particleCommon, TextureManager* textureManager, ModelManager* modelManager) {
	particleCommon_ = particleCommon;
	textureManager_ = textureManager;
	modelManager_ = modelManager;

	// 座標変換行列データ作成
	CreateTransformationMatrixData();

	// インスタンシングデータ作成
	CreateInstancingData();

	// Transform変数を作る
	transform_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	cameraTransform_ = {
	    {1.0f, 1.0f, 1.0f  },
        {0.3f, 0.0f, 0.0f  },
        {0.0f, 4.0f, -10.0f}
    };
	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}

void Particle::Update() {
	// fogの揺れ
	float elapsedTime = 1.0f / 60.0f;
	static float totalTime = 0.0f;
	totalTime += elapsedTime;

	timeParam_.time = totalTime;

	// Matrix4x4 cameraMatrix = MathUtility::MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	// Matrix4x4 viewMatrix = MathUtility::Inverse(cameraMatrix);
	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	projectionMatrix_ = MathUtility::MakePerspectiveFovMatrix(0.45f, static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight), 0.1f, 100.0f);
	transformMatrixData_->WVP = worldMatrix_ * viewMatrix_ * projectionMatrix_;
	transformMatrixData_->World = worldMatrix_;

	*transformMatrixData_ = {transformMatrixData_->WVP, transformMatrixData_->World};
	*directionalLightData_ = directionalLight_;
	*cameraForGPUData_ = cameraForGPU_;
	*fogParamData_ = fogParam_;
	*timeParamData_ = timeParam_;

	if (model_) {
		model_->Update();
	}
}

void Particle::Draw() {
	auto commadList = particleCommon_->GetDxCommon()->GetCommandList();

	// commandList->IASetIndexBuffer(&indexBufferView_); // IBVを設定
	// wvp用のBufferの場所を設定
	commadList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

	// 3Dモデルが割り当てられれいれば描画する
	if (model_) {
		model_->Draw();
	}
}

void Particle::SetModel(const std::string& filePath) {
	// モデルを検索してセットする
	model_ = modelManager_->FindModel(filePath);
}

void Particle::SetEnableFoging(const bool enableFoging) {
	if (model_) {
		model_->SetEnableFoging(enableFoging);
	}
}

void Particle::SetEnableLighting(const bool enableLighting) {
	if (model_) {
		model_->SetEnableLighting(enableLighting);
	}
}

void Particle::SetColor(Vector4 color) {
	if (model_) {
		model_->SetColor(color);
	}
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

void Particle::CreateTransformationMatrixData() {
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	wvpResource_ = CreateBufferResource(particleCommon_->GetDxCommon()->GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));
	wvpResource_->Unmap(0, nullptr);
	// 単位行列を書き込んでおく
	*transformMatrixData_ = {MathUtility::MakeIdentity4x4(), worldMatrix_};
}

void Particle::CreateInstancingData() {
	Transform transforms[kNumInstance];

	for (uint32_t index = 0; index < kNumInstance; ++index) {
		transforms[index].scale = {1.0f, 1.0f, 1.0f};
		transforms[index].rotate = {0.0f, 0.0f, 0.0f};
		transforms[index].translate = {index * 0.1f, index * 0.1f, index * 0.1f};
	}

	// Instancing用のTransformationMatrixリソースを作る
	instancingResource_ = CreateBufferResource(particleCommon_->GetDxCommon()->GetDevice(), sizeof(TransformationMatrix) * kNumInstance);

	// 書き込むためのアドレスを取得
	TransformationMatrix* instancingData = nullptr;
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));
	instancingResource_->Unmap(0, nullptr);

	for (uint32_t index = 0; index < kNumInstance; ++index) {
		Matrix4x4 worldMatrix = MathUtility::MakeAffineMatrix(transforms[index].scale, transforms[index].rotate, transforms[index].translate);
		Matrix4x4 viewProjectionMatrix = viewMatrix_ * projectionMatrix_;
		Matrix4x4 worldViewProjectionMatrix = MathUtility::Multiply(worldMatrix, viewProjectionMatrix);
		instancingData[index].WVP = worldViewProjectionMatrix;
		instancingData[index].World = worldMatrix;
	}
}
