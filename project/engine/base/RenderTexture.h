#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"

class RenderTexture {
public:
	// デストラクタ
	~RenderTexture();

	void Initialize(DirectXCommon* dx, SrvManager* srv, uint32_t width, uint32_t height);
	void BeginRender(DirectXCommon* dx);
	void EndRender(DirectXCommon* dx);

	uint32_t GetSRVIndexColor() const { return srvIndexColor_; }
	uint32_t GetSRVIndexDepth() const { return srvIndexDepth_; }

private:
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> resourceColor_;
	Microsoft::WRL::ComPtr<ID3D12Resource> resourceDepth_;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;

	uint32_t rtvIndex_ = 0;
	uint32_t dsvIndex_ = 0;

	uint32_t srvIndexColor_ = 0;
	uint32_t srvIndexDepth_ = 0;

	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};
};
