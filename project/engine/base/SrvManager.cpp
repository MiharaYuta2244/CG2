#include "SrvManager.h"

const uint32_t SrvManager::kMaxSRVCount = 512;

void SrvManager::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// DirectXCommonが既に作成しているSRVヒープを使う
	descriptorHeap_ = dxCommon_->GetSrvDescriptorHeap();

	// デスクリプタ1個分のサイズを取得して記録
	descriptorSize_ = dxCommon_->GetDescriptorSizeSRV();

	// 先頭ブロック(インデックス0)はImGuiなどが使っている想定なので、
	// 実際の割当は1から開始するようにする
	useIndex = 1;
}

uint32_t SrvManager::Allocate() {
	if (useIndex >= kMaxSRVCount) {
		assert(false);
	}

	// returnする番号を一旦記録しておく
	int index = useIndex;

	// 次回のために番号を1進める
	useIndex++;

	// 上で記録した番号をreturn
	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize_ * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize_ * index);
	return handleGPU;
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipLevels) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = mipLevels;
	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = structureByteStride;
	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateSRVforTextureCube(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipLevels) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MipLevels = mipLevels;
	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::PreDraw() {
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = {descriptorHeap_.Get()};
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT rootParameterIndex, uint32_t srvIndex) {
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(rootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}

bool SrvManager::CheckTextureMax() {
	return useIndex < kMaxSRVCount;
}