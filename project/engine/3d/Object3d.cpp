#include "Object3d.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "Model.h"
#include <DirectXMath.h>
#include <fstream>
#include <sstream>

using namespace Microsoft::WRL;
using namespace DirectX;

void Object3d::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// 座標変換行列データ作成
	CreateTransformationMatrixData();

	// CameraForGPU
	CreateCameraForGPUData();

	// フォグパラメータデータ作成
	CreateFogParamData();

	// タイムパラメータデータ作成
	CreateTimeParamData();

	// マテリアル作成
	CreateMaterialData();

	// Transform変数を作る
	transform_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// カメラをセットする
	camera_ = ctx_->object3dCommon->GetDefaultCamera();
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

	transformMatrixData_->WVP = modelData_.rootNode.localMatrix * worldViewProjectionMatrix_;
	transformMatrixData_->World = modelData_.rootNode.localMatrix * worldMatrix_;

	// 非均一スケールに対応した逆転置行列の計算
	// 3x3部分を抽出して逆行列を計算し、転置する
	Matrix4x4 worldInverseTranspose = MathUtility::MakeIdentity4x4();

	// ワールド行列の3x3部分を逆転置に変換
	float m00 = worldMatrix_.m[0][0], m01 = worldMatrix_.m[0][1], m02 = worldMatrix_.m[0][2];
	float m10 = worldMatrix_.m[1][0], m11 = worldMatrix_.m[1][1], m12 = worldMatrix_.m[1][2];
	float m20 = worldMatrix_.m[2][0], m21 = worldMatrix_.m[2][1], m22 = worldMatrix_.m[2][2];

	// 3x3行列式の計算
	float det = m00 * (m11 * m22 - m12 * m21) - m01 * (m10 * m22 - m12 * m20) + m02 * (m10 * m21 - m11 * m20);

	if (fabsf(det) > 1e-6f) {
		// 逆行列を計算
		float invDet = 1.0f / det;

		worldInverseTranspose.m[0][0] = (m11 * m22 - m12 * m21) * invDet;
		worldInverseTranspose.m[0][1] = (m02 * m21 - m01 * m22) * invDet;
		worldInverseTranspose.m[0][2] = (m01 * m12 - m02 * m11) * invDet;

		worldInverseTranspose.m[1][0] = (m12 * m20 - m10 * m22) * invDet;
		worldInverseTranspose.m[1][1] = (m00 * m22 - m02 * m20) * invDet;
		worldInverseTranspose.m[1][2] = (m02 * m10 - m00 * m12) * invDet;

		worldInverseTranspose.m[2][0] = (m10 * m21 - m11 * m20) * invDet;
		worldInverseTranspose.m[2][1] = (m01 * m20 - m00 * m21) * invDet;
		worldInverseTranspose.m[2][2] = (m00 * m11 - m01 * m10) * invDet;
	} else {
		// 行列式がゼロの場合は単位行列を使用
		worldInverseTranspose = MathUtility::MakeIdentity4x4();
	}

	transformMatrixData_->WorldInverseTranspose = worldInverseTranspose;

	*transformMatrixData_ = {transformMatrixData_->WVP, transformMatrixData_->World, transformMatrixData_->WorldInverseTranspose};

	// Object3dCommon更新
	ctx_->object3dCommon->Update();

	*cameraForGPUData_ = cameraForGPU_;
	*fogParamData_ = fogParam_;
	*timeParamData_ = timeParam_;

	if (materialData_) {
		*materialData_ = material_;
	}

	if (model_) {
		model_->Update();
	}
}

void Object3d::Draw() {
	// 3Dオブジェクト描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	ctx_->object3dCommon->DrawSettingCommon();

	auto commandList = ctx_->object3dCommon->GetDxCommon()->GetCommandList();

	// wvp用のBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	// --- インスタンスのマテリアルを先にセット（root b0） ---
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// CameraForGPUCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(4, cameraForGPUResource_->GetGPUVirtualAddress());
	// FogParamCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(5, fogParamResource_->GetGPUVirtualAddress());
	// TimeParamCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(6, timeParamResource_->GetGPUVirtualAddress());

	// 3Dモデルが割り当てられれば描画する
	if (model_) {
		model_->Draw();
	}
}

void Object3d::SetModel(const std::string& filePath) {
	// モデルを検索してセットする
	model_ = ctx_->modelManager->FindModel(filePath);

	// モデルが正常に読み込まれた場合、modelDataをコピー
	if (model_) {
		modelData_ = model_->GetModelData();
	}
}

void Object3d::SetColor(Vector4 color) {
	// 変更: モデル共有の material を直接変更しない。インスタンス側の material_ を変更する
	material_.color = color;
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
	wvpResource_ = CreateBufferResource(ctx_->object3dCommon->GetDxCommon()->GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));
	wvpResource_->Unmap(0, nullptr);
	// 単位行列を書き込んでおく
	*transformMatrixData_ = {MathUtility::MakeIdentity4x4(), worldMatrix_, MathUtility::Transpose(worldMatrix_)};
}

void Object3d::CreateCameraForGPUData() {
	// cameraForGPUのリソースを作る
	cameraForGPUResource_ = CreateBufferResource(ctx_->object3dCommon->GetDxCommon()->GetDevice(), sizeof(CameraForGPU));
	// アドレス取得
	cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData_));
	cameraForGPUResource_->Unmap(0, nullptr);
	cameraForGPU_ = {0.0f, 0.0f, -10.0f};
	// cameraForGPUDataへの書き込み
	*cameraForGPUData_ = cameraForGPU_;
}

void Object3d::CreateFogParamData() {
	// フォグパラメータのリソースを作る
	fogParamResource_ = CreateBufferResource(ctx_->object3dCommon->GetDxCommon()->GetDevice(), sizeof(FogParam));
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
	timeParamResource_ = CreateBufferResource(ctx_->object3dCommon->GetDxCommon()->GetDevice(), sizeof(TimeParam));
	// アドレス取得
	timeParamResource_->Map(0, nullptr, reinterpret_cast<void**>(&timeParamData_));
	timeParamResource_->Unmap(0, nullptr);
	// 初期化
	timeParam_.time = 1.0f / 60.0f;
	// timeParamDataへの書き込み
	*timeParamData_ = timeParam_;
}

void Object3d::CreateMaterialData() {
	// マテリアル用のリソースを作る
	materialResource_ = CreateBufferResource(ctx_->object3dCommon->GetDxCommon()->GetDevice(), sizeof(Material));
	// アドレス取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialResource_->Unmap(0, nullptr);
	// 初期化
	material_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	material_.enableLighting = true;
	material_.enableFoging = false;
	material_.uvTransform = MathUtility::MakeIdentity4x4();
	material_.shininess = 64.0f;
	// GPUへ書き込み
	*materialData_ = material_;
}
