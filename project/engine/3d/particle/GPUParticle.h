#pragma once
#include "DirectXCommon.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Vector4.h"
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl/client.h>

struct ParticleCS {
	Vector3 translate;
	Vector3 scale;
	float lifeTime;
	Vector3 velocity;
	float currentTime;
	Vector4 color;
};

struct PerView {
	Matrix4x4 viewprojection;
	Matrix4x4 billboardMatrix;
};

class GPUParticle {
public:
	static const uint32_t kMaxParticles = 1024;

	/// <summary>
	/// 初期化関数
	/// </summary>
	/// <param name="dxCommon">DirectX共通クラスのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewData"></param>
	void Update(const PerView& viewData);

	/// <summary>
	/// コンピュートシェーダーによる初期化の実行
	/// </summary>
	void DispatchInitialize();

	/// <summary>
	/// 描画設定とインスタンス描画の実行
	/// </summary>
	void Draw();

private:
	void InitializeShaderCompiler();
	void CreateComputePipeline();
	void CreateGraphicsPipeline();
	void CreateResources();

private:
	DirectXCommon* dxCommon_ = nullptr;

	// ShaderCompiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	// ComputePipeline (初期化・更新用)
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipelineState_;

	// GraphicsPipeline (描画用)
	Microsoft::WRL::ComPtr<ID3D12RootSignature> graphicsRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> particleBuffer_; // StructuredBuffer (SRV/UAV兼用)
	Microsoft::WRL::ComPtr<ID3D12Resource> perViewBuffer_;  // ConstantBuffer (CBV)
	PerView* mappedPerView_ = nullptr;

	// ディスクリプタヒープとハンドル
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvUavHeap_;
	D3D12_GPU_DESCRIPTOR_HANDLE particleUAVHeapHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE particleSRVHeapHandle{};

	// 現在のリソース状態管理
	D3D12_RESOURCE_STATES particleState_ = D3D12_RESOURCE_STATE_COMMON;
};