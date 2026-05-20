#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <dxcapi.h>
#include <string>
#include <wrl.h>

/// <summary>
/// 適用するPixelShaderのタイプ
/// </summary>
enum class PostEffectType {
	Fullscreen,
	Grayscale,
	Sepia,
	Vignette,
	Smoothing,
	Gaussian,
	Outline,
	RadialBlur,
	Dissolve,
	Random,
};

struct VignetteParam {
	Vector4 color;
	float intensity;
};

namespace TinyEngine {
class CopyImage {
public:
	// 初期化
	void Initialize(DirectXCommon* dx, PostEffectType type);

	// 描画
	void Draw(DirectXCommon* dx, SrvManager* srv, uint32_t srvIndex);

private:
	// シェーダコンパイラの初期化
	void InitializeShaderCompiler();

	// グラフィックスパイプライン作成
	void CreateGraphicsPipeline(DirectXCommon* dx);

	// ヴィネットのリソース作成　データ書き込み
	void CreateVignetteCB();

private:
	// DxCommon
	DirectXCommon* dxCommon_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

	// Shader compiler objects
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	// 適用するシェーダー
	PostEffectType postEffectType_ = PostEffectType::Fullscreen;

	// シェーダーのマップ
	std::unordered_map<PostEffectType, std::wstring> shaderMap_ = {
	    {PostEffectType::Fullscreen, L"Fullscreen"},
        {PostEffectType::Grayscale,  L"Grayscale" },
        {PostEffectType::Sepia,      L"Sepia"     },
        {PostEffectType::Vignette,   L"Vignette"  },
	    {PostEffectType::Smoothing,  L"Smoothing" },
        {PostEffectType::Gaussian,   L"Gaussian"  },
        {PostEffectType::Outline,    L"Outline"   },
        {PostEffectType::RadialBlur, L"RadialBlur"},
	    {PostEffectType::Dissolve,   L"Dissolve"  },
        {PostEffectType::Random,     L"Random"    },
	};

	// ヴィネットのCB
	Microsoft::WRL::ComPtr<ID3D12Resource> vignetteCB_;
	VignetteParam* vignetteData_;
	VignetteParam vignetteParam_;
};
} // namespace TinyEngine