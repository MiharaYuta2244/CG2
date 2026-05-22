#pragma once
#include "DirectXCommon.h"
#include "PostEffectTypes.h"
#include "SrvManager.h"
#include <dxcapi.h>
#include <string>
#include <wrl.h>

/// <summary>
/// 適用するPixelShaderのタイプ
/// </summary>
enum class PostEffectType {
	FullScreen,
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

	// CB作成
	void CreateCB();

	// 各種パラメータの初期設定
	void AllParamSetting();

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
	PostEffectType postEffectType_ = PostEffectType::FullScreen;

	// シェーダーのマップ
	std::unordered_map<PostEffectType, std::wstring> shaderMap_ = {
	    {PostEffectType::FullScreen, L"Fullscreen"},
        {PostEffectType::Grayscale,  L"Grayscale" },
        {PostEffectType::Sepia,      L"Sepia"     },
        {PostEffectType::Vignette,   L"Vignette"  },
	    {PostEffectType::Smoothing,  L"BoxFilter" },
        {PostEffectType::Gaussian,   L"Gaussian"  },
        {PostEffectType::Outline,    L"Outline"   },
        {PostEffectType::RadialBlur, L"RadialBlur"},
	    {PostEffectType::Dissolve,   L"Dissolve"  },
        {PostEffectType::Random,     L"Random"    },
	};

	// 各種パラメータ
	VignetteParam vignetteParam_;
	GrayscaleParam grayscaleParam_;
	SepiaParam sepiaParam_;
	SmoothingParam smoothingParam_;

	// CB Size Map
	std::unordered_map<PostEffectType, size_t> cbSizeMap_ = {
	    {PostEffectType::FullScreen, 0                     }, // パラメータ不要
	    {PostEffectType::Vignette,   sizeof(VignetteParam) },
	    {PostEffectType::Grayscale,  sizeof(GrayscaleParam)},
	    {PostEffectType::Sepia,      sizeof(SepiaParam)    },
	    {PostEffectType::Smoothing,  sizeof(SmoothingParam)},
	    //{PostEffectType::Gaussian,   sizeof(GaussianParam)  },
	    //{PostEffectType::Outline,    sizeof(OutlineParam)   },
	    //{PostEffectType::RadialBlur, sizeof(RadialBlurParam)},
	    //{PostEffectType::Dissolve,   sizeof(DissolveParam)  },
	    //{PostEffectType::Random,     sizeof(RandomParam)    },
	};

	// Param Ptr Map
	std::unordered_map<PostEffectType, void*> paramPtrMap_ = {
	    {PostEffectType::Vignette,  &vignetteParam_ },
        {PostEffectType::Grayscale, &grayscaleParam_},
        {PostEffectType::Sepia,     &sepiaParam_    },
        {PostEffectType::Smoothing, &smoothingParam_},
	    /*{PostEffectType::Gaussian,   &gaussianParam_  },
	    {PostEffectType::Outline,    &outlineParam_   },
	    {PostEffectType::RadialBlur, &radialBlurParam_},
	    {PostEffectType::Dissolve,   &dissolveParam_  },
	    {PostEffectType::Random,     &randomParam_    },*/
	};

	// CB
	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;

	// Data
	void* cbData_ = nullptr;
};
} // namespace TinyEngine