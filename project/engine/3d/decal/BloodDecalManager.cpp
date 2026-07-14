#include "BloodDecalManager.h"
#include "DirectXUtils.h"
#include "TextureManager.h"
#include "ParticleCommon.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace TinyEngine;

BloodDecalManager::~BloodDecalManager() {
	if (ctx_ && ctx_->srvManager) {
		ctx_->srvManager->Free(srvIndex_);
	}
}

void BloodDecalManager::Initialize(EngineContext* ctx, const std::string& texturePath) {
	ctx_ = ctx;

	// インスタンシング用リソースとSRVの作成
	CreateInstancingResource();
	srvIndex_ = ctx_->srvManager->Allocate();
	CreateInstancingSRV(srvIndex_);

	// Quadモデルデータの生成
	modelData_ = CreatePrimitive(texturePath);

	// 頂点データの初期化
	CreateVertexData();

	// テクスチャ読み込み
	ctx_->textureManager->LoadTexture(texturePath);
	modelData_.material.textureIndex = ctx_->textureManager->GetSrvIndex(texturePath);

	// 配列のメモリをあらかじめ最大数分確保しておく
	bloodList_.reserve(kNumMaxInstance);

	// マテリアルのCBVを作成
	UINT materialBufferSize = (sizeof(ParticleMaterial) + 255) & ~255;
	materialResource_ = DirectXUtils::CreateBufferResource(ctx_->particleCommon->GetDxCommon()->GetDevice(), materialBufferSize);
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	material_.color = {1, 1, 1, 1};
	material_.uvTransform = MathUtility::MakeIdentity4x4();
	material_.alphaCutoff = 0.0f;
	*materialData_ = material_;
}

void BloodDecalManager::AddBlood(const Vector3& position, const Vector3& rotation, const Vector3& scale, const Vector4& color) {
	BloodData newData;
	newData.transform = {scale, rotation, position};
	newData.color = color;

	// 追加時にワールド行列を一度だけ計算してキャッシュしておく
	newData.worldMatrix = MathUtility::MakeAffineMatrix(scale, rotation, position);

	// リングバッファ形式で追加・上書き
	if (bloodList_.size() < kNumMaxInstance) {
		bloodList_.push_back(newData);
	} else {
		uint32_t overwriteIndex = bloodIndex_ % kNumMaxInstance;
		bloodList_[overwriteIndex] = newData;
	}

	bloodIndex_++;
	numInstance_ = static_cast<uint32_t>(bloodList_.size());
}

void BloodDecalManager::Update() {
	if (numInstance_ == 0 || camera_ == nullptr)
		return;

	// カメラのビュープロジェクション行列を取得
	Matrix4x4 viewProjMatrix = camera_->GetViewProjectionMatrix();

	// GPUに送るデータへ書き込み
	for (uint32_t i = 0; i < numInstance_; ++i) {
		instancingData_[i].World = bloodList_[i].worldMatrix;
		// キャッシュされたワールド行列にカメラの行列を掛けるだけ
		instancingData_[i].WVP = MathUtility::Multiply(bloodList_[i].worldMatrix, viewProjMatrix);
		instancingData_[i].color = bloodList_[i].color;
	}
}

void BloodDecalManager::Draw() {
	if (numInstance_ <= 0)
		return;

	// ParticleCommon等のパイプライン設定を流用
	ctx_->particleCommon->DrawSettingCommon();

	auto commandList = ctx_->particleCommon->GetDxCommon()->GetCommandList();

	// マテリアル
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// StructuredBufferの場所を設定
	commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU_);

	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// ピクセルシェーダー用テクスチャの設定
	commandList->SetGraphicsRootDescriptorTable(2, ctx_->textureManager->GetSrvHandleGPU(modelData_.material.textureFilePath));

	// 描画
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), numInstance_, 0, 0);
}

ModelData BloodDecalManager::CreatePrimitive(const std::string& texturePath) {
	ModelData modelData;

	// XZ平面
	modelData.vertices.push_back({
	    {-1.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    });
	modelData.vertices.push_back({
	    {1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    });
	modelData.vertices.push_back({
	    {-1.0f, -1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}
    });
	modelData.vertices.push_back({
	    {-1.0f, -1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}
    });
	modelData.vertices.push_back({
	    {1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    });
	modelData.vertices.push_back({
	    {1.0f, -1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}
    });

	modelData.material.textureFilePath = "resources/textures/" + texturePath;
	return modelData;
}

void BloodDecalManager::CreateVertexData() {
	vertexResource_ = DirectXUtils::CreateBufferResource(ctx_->particleCommon->GetDxCommon()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Unmap(0, nullptr);
}

void BloodDecalManager::CreateInstancingResource() {
	// 10000個分のStructuredBufferリソースを作成
	instancingResource_ = DirectXUtils::CreateBufferResource(ctx_->particleCommon->GetDxCommon()->GetDevice(), sizeof(BloodDecalForGPU) * kNumMaxInstance);
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
}

void BloodDecalManager::CreateInstancingSRV(UINT srvIndex) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = kNumMaxInstance;
	srvDesc.Buffer.StructureByteStride = sizeof(BloodDecalForGPU);

	instancingSrvHandleCPU_ = ctx_->srvManager->GetCPUDescriptorHandle(srvIndex);
	instancingSrvHandleGPU_ = ctx_->srvManager->GetGPUDescriptorHandle(srvIndex);
	ctx_->particleCommon->GetDxCommon()->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &srvDesc, instancingSrvHandleCPU_);
}