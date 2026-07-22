#include "DecalManager.h"
#include "DirectXUtils.h"
#include "ParticleCommon.h"
#include "TextureManager.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace TinyEngine;

DecalManager::~DecalManager() {
	if (ctx_ && ctx_->srvManager) {
		// 全てのグループのSRVを解放
		for (auto& pair : decalGroups_) {
			ctx_->srvManager->Free(pair.second->srvIndex);
		}
	}
}

void DecalManager::Initialize(EngineContext* ctx) {
	ctx_ = ctx;
	srvIndex_ = ctx_->srvManager->Allocate();

	// Quadモデルデータの生成
	modelData_ = CreatePrimitive();

	// 頂点データの初期化
	CreateVertexData();

	// 配列のメモリをあらかじめ最大数分確保しておく
	decalList_.reserve(kNumMaxInstance);

	// マテリアルのCBVを作成
	UINT materialBufferSize = (sizeof(ParticleMaterial) + 255) & ~255;
	materialResource_ = DirectXUtils::CreateBufferResource(ctx_->particleCommon->GetDxCommon()->GetDevice(), materialBufferSize);
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	material_.color = {1, 1, 1, 1};
	material_.uvTransform = MathUtility::MakeIdentity4x4();
	material_.alphaCutoff = 0.0f;
	*materialData_ = material_;
}

DecalManager::DecalData* DecalManager::AddDecal(const std::string& texturePath, const Vector3& position, const Vector3& rotation, const Vector3& scale, const Vector4& color) {
	std::string fullPath = "resources/textures/" + texturePath;
	ctx_->textureManager->LoadTexture(texturePath);

	// グループが存在しない場合は新規作成し、個別のGPUバッファとSRVを確保する
	if (decalGroups_.find(fullPath) == decalGroups_.end()) {
		auto group = std::make_unique<DecalGroup>();
		group->decalList.reserve(kNumMaxInstance);

		group->instancingResource = DirectXUtils::CreateBufferResource(ctx_->particleCommon->GetDxCommon()->GetDevice(), sizeof(DecalForGPU) * kNumMaxInstance);
		group->instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&group->instancingData));

		group->srvIndex = ctx_->srvManager->Allocate();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = kNumMaxInstance;
		srvDesc.Buffer.StructureByteStride = sizeof(DecalForGPU);

		group->instancingSrvHandleCPU = ctx_->srvManager->GetCPUDescriptorHandle(group->srvIndex);
		group->instancingSrvHandleGPU = ctx_->srvManager->GetGPUDescriptorHandle(group->srvIndex);
		ctx_->particleCommon->GetDxCommon()->GetDevice()->CreateShaderResourceView(group->instancingResource.Get(), &srvDesc, group->instancingSrvHandleCPU);

		decalGroups_[fullPath] = std::move(group);
	}

	DecalData newData;
	newData.transform = {scale, rotation, position};
	newData.color = color;
	newData.worldMatrix = MathUtility::MakeAffineMatrix(scale, rotation, position);

	auto& group = decalGroups_[fullPath];
	DecalData* createdDecal = nullptr;

	// グループごとのリングバッファ形式で追加・上書き
	if (group->decalList.size() < kNumMaxInstance) {
		group->decalList.push_back(newData);
		createdDecal = &group->decalList.back();
	} else {
		uint32_t overwriteIndex = group->decalIndex % kNumMaxInstance;
		group->decalList[overwriteIndex] = newData;
		createdDecal = &group->decalList[overwriteIndex];
	}

	group->decalIndex++;
	return createdDecal;
}

void DecalManager::Update() {
	if (camera_ == nullptr)
		return;

	// カメラのビュープロジェクション行列を取得
	Matrix4x4 viewProjMatrix = camera_->GetViewProjectionMatrix();

	// GPUに送るデータへ書き込み
	for (auto& pair : decalGroups_) {
		auto& group = pair.second;
		uint32_t numInstance = static_cast<uint32_t>(group->decalList.size());

		for (uint32_t i = 0; i < numInstance; ++i) {
			// transformの変更を反映するためワールド行列を再計算
			group->decalList[i].worldMatrix = MathUtility::MakeAffineMatrix(group->decalList[i].transform.scale, group->decalList[i].transform.rotate, group->decalList[i].transform.translate);

			group->instancingData[i].World = group->decalList[i].worldMatrix;
			group->instancingData[i].WVP = MathUtility::Multiply(group->decalList[i].worldMatrix, viewProjMatrix);
			group->instancingData[i].color = group->decalList[i].color;
		}
	}
}

void DecalManager::Draw() {
	if (decalGroups_.empty())
		return;

	// ParticleCommon等のパイプライン設定を流用
	ctx_->particleCommon->DrawSettingCommon();

	auto commandList = ctx_->particleCommon->GetDxCommon()->GetCommandList();

	// マテリアル
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	for (auto& pair : decalGroups_) {
		const std::string& texturePath = pair.first;
		auto& group = pair.second;
		uint32_t numInstance = static_cast<uint32_t>(group->decalList.size());

		if (numInstance <= 0)
			continue;

		// StructuredBufferの場所を設定
		commandList->SetGraphicsRootDescriptorTable(1, group->instancingSrvHandleGPU);

		// ピクセルシェーダー用テクスチャの設定
		commandList->SetGraphicsRootDescriptorTable(2, ctx_->textureManager->GetSrvHandleGPU(texturePath));

		// 描画
		commandList->DrawInstanced(UINT(modelData_.vertices.size()), numInstance, 0, 0);
	}
}

ModelData DecalManager::CreatePrimitive() {
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

	return modelData;
}

void DecalManager::CreateVertexData() {
	vertexResource_ = DirectXUtils::CreateBufferResource(ctx_->particleCommon->GetDxCommon()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Unmap(0, nullptr);
}