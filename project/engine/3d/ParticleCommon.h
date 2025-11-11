#pragma once
#include "DirectXCommon.h"
#include "DebugCamera.h"

class ParticleCommon {
public:
	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingCommon();

	void Initialize(DirectXCommon* dxCommon);

	void SetDefaultCamera(DebugCamera* camera) { defaultCamera_ = camera; }

	// getter
	DirectXCommon* GetDxCommon() const { return dxCommon_; }

	ID3D12Resource* GetInstancingResource() const { instancingResource_.Get(); }

	D3D12_GPU_DESCRIPTOR_HANDLE GetInstancingSrvGpuHandle() const { return instancingSrvHandleGPU_; }

	DebugCamera* GetDefaultCamera() { return defaultCamera_; }

private:
	/// <summary>
	/// ルートシグネチャの作成
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// グラフィックスパイプラインの生成
	/// </summary>
	void CreateGraphicsPipeline();

	void InitializeShaderCompiler();

	// コンパイルシェーダー
	IDxcBlob* CompileShader(
	    // CompilerするShaderファイルへのパス
	    const std::wstring& filePath,
	    // Compilerに使用するProfile
	    const wchar_t* profile,
	    // 初期化で生成したものを3つ
	    IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);

private:
	DirectXCommon* dxCommon_;

	// Root Signature / Pipeline
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// Shader Compiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	// Resource
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	 // インスタンシング用SRVのDescriptorハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_ = {};
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_ = {};

	// カメラ
	DebugCamera* defaultCamera_ = nullptr;
};
