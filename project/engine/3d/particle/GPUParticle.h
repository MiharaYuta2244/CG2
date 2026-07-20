#pragma once
#include "DirectXCommon.h"
#include "EngineContext.h"
#include "Matrix4x4.h"
#include "ParticleMaterial.h"
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

struct PerFrame {
	float time;
	float deltaTime;
};

struct EmitterSphere {
	Vector3 translate;
	float radius;
	uint32_t count;
	float frequency;
	float frequencyTime;
	uint32_t emit;
	uint32_t seed;
};

class GPUParticle {
public:
	static const uint32_t kMaxParticles = 1024;

	~GPUParticle();

	/// <summary>
	/// 初期化関数
	/// </summary>
	/// <param name="dxCommon">DirectX共通クラスのポインタ</param>
	void Initialize(EngineContext* ctx, const std::string& texturepath);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewData"></param>
	void Update(const PerView& viewData, float deltaTime);

	/// <summary>
	/// コンピュートシェーダーによる初期化の実行
	/// </summary>
	void DispatchInitialize();

	/// <summary>
	/// 描画設定とインスタンス描画の実行
	/// </summary>
	void Draw();

	/// <summary>
	/// エミッター用Dispatch
	/// </summary>
	void DispatchEmit();

	/// <summary>
	/// 更新用Dispatch
	/// </summary>
	void DispatchUpdate();

private:
	void InitializeShaderCompiler();
	void CreateComputePipeline();
	void CreateGraphicsPipeline();
	void CreateResources();
	void CreateEmitComputePipeline();
	void CreateUpdateComputePipeline();

private:
	EngineContext* ctx_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
	std::string texturePath_;
	std::string textureFullPath_;

	// ShaderCompiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	// ComputePipeline (初期化・更新用)
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipelineState_;

	// ComputePipeline (射出用)
	Microsoft::WRL::ComPtr<ID3D12RootSignature> emitRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> emitPipelineState_;

	// GraphicsPipeline (描画用)
	Microsoft::WRL::ComPtr<ID3D12RootSignature> graphicsRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// GraphicsPipeline (更新用)
	Microsoft::WRL::ComPtr<ID3D12RootSignature> updateRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> updatePipelineState_;

	// リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> particleBuffer_; // StructuredBuffer (SRV/UAV兼用)
	Microsoft::WRL::ComPtr<ID3D12Resource> perViewBuffer_;  // ConstantBuffer (CBV)
	PerView* mappedPerView_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer_; // ConstantBuffer (CBV)
	ParticleMaterial* mappedMaterial_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> texture_; // テクスチャ本体
	D3D12_GPU_DESCRIPTOR_HANDLE textureSRVHeapHandle{};
	Microsoft::WRL::ComPtr<ID3D12Resource> emitterBuffer_;
	EmitterSphere* mappedEmitter_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> perFrameBuffer_;
	PerFrame* mappedPerFrame_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> freeListIndexBuffer_;
	uint32_t freeListIndexUavIndex_ = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> freeListBuffer_;
	uint32_t freeListUavIndex_ = 0;

	// ディスクリプタヒープとハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE particleUAVHeapHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE particleSRVHeapHandle{};

	// 現在のリソース状態管理
	D3D12_RESOURCE_STATES particleState_ = D3D12_RESOURCE_STATE_COMMON;

	uint32_t particleUavIndex_ = 0;
	uint32_t particleSrvIndex_ = 0;

	// エミッター
	EmitterSphere emitterSphere_;

	PerFrame perFrame_;
};