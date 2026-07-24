#pragma once
#include "DirectXCommon.h"

/// <summary>
/// SRVマネージャークラス
/// </summary>
class SrvManager {
public:
	void Initialize(DirectXCommon* dxCommon);

	uint32_t Allocate();
	void Free(uint32_t index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	// SRV生成(テクスチャ用)
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipLevels);

	// SRV生成(Structured Buffer用)
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

	// SRV生成(CubeMap用)
	void CreateSRVforTextureCube(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipLevels);

	void PreDraw();

	void SetGraphicsRootDescriptorTable(UINT rootParameterIndex, uint32_t srvIndex);

	// テクスチャ枚数上限チェック
	bool CheckTextureMax();

	// ImGui用　ヒープとインデックス逆引き
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const { return descriptorHeap_; }
	uint32_t GetIndexFromHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) const;

private:
	DirectXCommon* dxCommon_ = nullptr;

	// 最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;

	// SRV用のデスクリプタサイズ
	uint32_t descriptorSize_;

	// SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

	// 次に使用するSRVインデックス
	uint32_t useIndex = 1;

	// 解放されたインデックスを保持するリスト
	std::vector<uint32_t> freeIndices_;
};
