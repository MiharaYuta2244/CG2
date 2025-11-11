#include <DirectXMath.h>
#include <fstream>
#include <sstream>
#include "Object3d.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "Model.h"


using namespace Microsoft::WRL;
using namespace DirectX;

void Object3d::Initialize(Object3dCommon* modelCommon, TextureManager* textureManager, ModelManager* modelManager) {
	object3dCommon_ = modelCommon;
	textureManager_ = textureManager;
	modelManager_ = modelManager;

	// 座標変換行列データ作成
	CreateTransformationMatrixData();

	// 平行光源データ作成
	CreateDirectionalLightData();

	// CameraForGPU
	CreateCameraForGPUData();

	// フォグパラメータデータ作成
	CreateFogParamData();

	// タイムパラメータデータ作成
	CreateTimeParamData();

	// Transform変数を作る
	transform_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// カメラをセットする
	camera_ = object3dCommon_->GetDefaultCamera();
}

void Object3d::Update() {
	// fogの揺れ
	float elapsedTime = 1.0f / 60.0f;
	static float totalTime = 0.0f;
	totalTime += elapsedTime;

	timeParam_.time = totalTime;

	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix_ = MathUtility::Multiply(worldMatrix_, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix_ = worldMatrix_;
	}

	transformMatrixData_->WVP = worldViewProjectionMatrix_;
	transformMatrixData_->World = worldMatrix_;
	transformMatrixData_->WorldInverseTranspose = MathUtility::Transpose(worldMatrix_);

	*transformMatrixData_ = {transformMatrixData_->WVP, transformMatrixData_->World, transformMatrixData_->WorldInverseTranspose};
	*directionalLightData_ = directionalLight_;
	*cameraForGPUData_ = cameraForGPU_;
	*fogParamData_ = fogParam_;
	*timeParamData_ = timeParam_;

	if (model_) {
		model_->Update();
	}
}

void Object3d::Draw() {
	// 3Dオブジェクト描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	object3dCommon_->DrawSettingCommon();

	auto commadList = object3dCommon_->GetDxCommon()->GetCommandList();

	// commandList->IASetIndexBuffer(&indexBufferView_); // IBVを設定
	// wvp用のBufferの場所を設定
	commadList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	// ライティングCBufferの場所を指定
	commadList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	// CameraForGPUCBufferの場所を指定
	commadList->SetGraphicsRootConstantBufferView(4, cameraForGPUResource_->GetGPUVirtualAddress());
	// FogParamCBufferの場所を指定
	commadList->SetGraphicsRootConstantBufferView(5, fogParamResource_->GetGPUVirtualAddress());
	// TimeParamCBufferの場所を指定
	commadList->SetGraphicsRootConstantBufferView(6, timeParamResource_->GetGPUVirtualAddress());

	// 3Dモデルが割り当てられれいれば描画する
	if (model_) {
		model_->Draw();
	}
}

void Object3d::SetModel(const std::string& filePath) {
	// モデルを検索してセットする
	model_ = modelManager_->FindModel(filePath);
}

void Object3d::SetEnableFoging(const bool enableFoging) { 
	if (model_) {
		model_->SetEnableFoging(enableFoging);
	}
}

void Object3d::SetEnableLighting(const bool enableLighting) { 
	if (model_) {
		model_->SetEnableLighting(enableLighting);
	}
}

void Object3d::SetColor(Vector4 color) {
	if (model_) {
		model_->SetColor(color);
	}
}

ComPtr<ID3D12Resource> Object3d::CreateBufferResource(ComPtr<ID3D12Device> device, size_t sizeBytes) {
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

void Object3d::CreateTransformationMatrixData() {
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	wvpResource_ = CreateBufferResource(object3dCommon_->GetDxCommon()->GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));
	wvpResource_->Unmap(0, nullptr);
	// 単位行列を書き込んでおく
	*transformMatrixData_ = {MathUtility::MakeIdentity4x4(), worldMatrix_, MathUtility::Transpose(worldMatrix_)};
}

void Object3d::CreateDirectionalLightData() {
	// 平行光源用のリソースを作る
	directionalLightResource_ = CreateBufferResource(object3dCommon_->GetDxCommon()->GetDevice(), sizeof(DirectionalLight));
	// アドレス取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightResource_->Unmap(0, nullptr);
	// 初期化
	directionalLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.0f};
	directionalLight_.intensity = 0.0f;

	// Vector3 → XMVECTOR 変換
	XMVECTOR dirVec = XMVectorSet(
	    directionalLight_.direction.x, directionalLight_.direction.y, directionalLight_.direction.z,
	    0.0f // ← w成分は不要なので0
	);
	// 正規化
	dirVec = XMVector3Normalize(dirVec);
	// XMVECTOR → Vector3 に戻す
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&directionalLight_.direction), dirVec);

	// directionalLightDataへの書き込み
	*directionalLightData_ = directionalLight_;
}

void Object3d::CreateCameraForGPUData() {
	// cameraForGPUのリソースを作る
	cameraForGPUResource_ = CreateBufferResource(object3dCommon_->GetDxCommon()->GetDevice(), sizeof(CameraForGPU));
	// アドレス取得
	cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData_));
	cameraForGPUResource_->Unmap(0, nullptr);
	cameraForGPU_ = {0.0f, 0.0f, -10.0f};
	// cameraForGPUDataへの書き込み
	*cameraForGPUData_ = cameraForGPU_;
}

void Object3d::CreateFogParamData() {
	// フォグパラメータのリソースを作る
	fogParamResource_ = CreateBufferResource(object3dCommon_->GetDxCommon()->GetDevice(), sizeof(FogParam));
	// アドレス取得
	fogParamResource_->Map(0, nullptr, reinterpret_cast<void**>(&fogParamData_));
	fogParamResource_->Unmap(0, nullptr);
	// 初期化
	fogParam_.fogCenter = {0.0f, 0.0f, 0.0f};
	fogParam_.fogColor = {1.0f, 1.0f, 1.0f};
	fogParam_.fogIntensity = 0.5f;
	fogParam_.radius = 20.0f;
	// fogParamDataへの書き込み
	*fogParamData_ = fogParam_;
}

void Object3d::CreateTimeParamData() {
	// タイムパラメータのリソースを作る
	timeParamResource_ = CreateBufferResource(object3dCommon_->GetDxCommon()->GetDevice(), sizeof(TimeParam));
	// アドレス取得
	timeParamResource_->Map(0, nullptr, reinterpret_cast<void**>(&timeParamData_));
	timeParamResource_->Unmap(0, nullptr);
	// 初期化
	timeParam_.time = 1.0f / 60.0f;
	// timeParamDataへの書き込み
	*timeParamData_ = timeParam_;
}
