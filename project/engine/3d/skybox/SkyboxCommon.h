#pragma once
#include "DirectXCommon.h"
#include <wrl.h>

/// <summary>
/// Skyboxの共通クラス
/// </summary>
class SkyboxCommon {
public:
	void Initialize(DirectXCommon* dxCommon);
	void DrawSettingCommon();
	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

private:
	void CreateRootSignature();
	void CreateGraphicsPipeline();
	void InitializeShaderCompiler();

private:
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// ShaderCompiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};
