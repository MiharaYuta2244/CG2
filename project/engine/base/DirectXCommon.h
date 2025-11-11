#pragma once
class WinApp; // 前方宣言
#include <chrono>
#include "WinApp.h"
#include "Logger.h"
#include <memory>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <wrl.h>
#pragma comment(lib, "dxguid.lib")
#include <dxcapi.h>

class DirectXCommon {
public:
	DirectXCommon();
	~DirectXCommon();

	void Initialize(std::shared_ptr<WinApp> winApp);
	void BeginFrame();
	void EndFrame();
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device_; }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSrvDescriptorHeap() const { return srvDescriptorHeap_; }
	std::shared_ptr<WinApp> GetWinApp() const { return winApp_; }
	UINT GetSwapChainDescBufferCount() const { return swapChainDesc_.BufferCount; }
	DXGI_FORMAT GetRtvFormat() const { return rtvDesc_.Format; }
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList_; }
	HANDLE GetFenceEvent() const { return fenceEvent_; }
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetGraphicsPipelineState() { return graphicsPipelineState_; }
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() { return rootSignature_; }

	// Getter
	uint32_t GetDescriptorSizeSRV() const { return descriptorSizeSRV_; }
	uint32_t GetDescriptorSizeRTV() const { return descriptorSizeRTV_; }
	uint32_t GetDescriptorSizeDSV() const { return descriptorSizeDSV_; }

	/// <summary>
	/// SRVの指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	/// <param name="index">指定番号</param>
	/// <returns>CPUディスクリプタハンドル</returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRVの指定番号のGPUディスクリプタハンドルを取得する
	/// </summary>
	/// <param name="index">指定番号</param>
	/// <returns>GPUディスクリプタハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescripterHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

public:
	// 最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;

private:
	void InitializeDXGIDevice();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeaps();
	void CreateRTV();
	void InitializeShaderCompiler();
	void CreateDSV();
	void CreateViewport();
	void CreateScissor();
	void CreateFence();

	IDxcBlob* CompileShader(
	    // CompilerするShaderファイルへのパス
	    const std::wstring& filePath,
	    // Compilerに使用するProfile
	    const wchar_t* profile,
	    // 初期化で生成したものを3つ
	    IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	// FPS固定初期化
	void InitializeFixFPS();

	// FPS固定更新
	void UpdateFixFPS();

	// 記録時間(FPS固定)
	std::chrono::steady_clock::time_point reference_;

private:

	std::shared_ptr<WinApp> winApp_;

	// DXGI
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

	// D3D12 Core
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	// Descriptor Heaps
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;

	// SwapChain Resources
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2];
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2] = {};

	// Depth Buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	// Shader Compiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	// Root Signature / Pipeline
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// Viewport / Scissor
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};

	// Sync
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_ = 0;
	HANDLE fenceEvent_ = nullptr;

	UINT backBufferIndex_ = 0;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_;

	// DescriptorSize
	uint32_t descriptorSizeSRV_;
	uint32_t descriptorSizeRTV_;
	uint32_t descriptorSizeDSV_;

	// Debug
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_;

#ifdef _DEBUG
	void SetupDebugLayer();
#endif
};
