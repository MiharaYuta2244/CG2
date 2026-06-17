#pragma once
#include "DirectXCommon.h"
#include "PostEffectTypes.h"
#include "SrvManager.h"
#include "TextureManager.h"
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
	LuminanceOutline,
	DepthOutline,
	RadialBlur,
	Dissolve,
	Random,
	Glitch,
};

struct EffectMeta {
	std::wstring shaderName;
	size_t cbSize;
	void* paramPtr;
};

namespace TinyEngine {
class CopyImage {
public:
	// 初期化
	void Initialize(DirectXCommon* dx, PostEffectType type, TextureManager* textureManager);

	// 描画
	void Draw(DirectXCommon* dx, SrvManager* srv, uint32_t srvIndex, uint32_t depthSrvIndex);

	// ImGui
	void DrawImGui();

	// カメラのVP行列Setter
	void SetProjectionInverse(const Matrix4x4& projInv) { depthOutlineParam_.projectionInverse = projInv; }

	// VignetteのIntensityのSetter
	void SetVignetteIntensity(float intensity) { vignetteParam_.intensity = intensity; }

	// VignetteのColorのSetter
	void SetVignetteColor(Vector4 color) { vignetteParam_.color = color; }

	// RandomShader用に経過時間を設定
	void SetRandomTime(float time) { randomParam_.time = time; }

	// GlitchノイズのIntensityのSetter
	void SetGlitchIntensity(float intensity) { glitchParam_.intensity = intensity; }

	// GlitchShader用に経過時間を設定
	void SetGlitchTime(float time) { glitchParam_.time = time; }

	// RadialBlur用にWidthを設定
	void SetRadialBlurWidth(float blurWidth) { radialBlurParam_.blurWidth = blurWidth; }

	// RadialBlur用にNumnumSamplesを設定
	void SetRadialBlurNumSamples(float numSamples) { radialBlurParam_.numSamples = numSamples; }

	// Dissolveのパラメータ設定
	void SetDissolveParam(DissolveParam dissolveParam) { dissolveParam_ = dissolveParam; }

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

	// 各種パラメータ
	VignetteParam vignetteParam_;
	GrayscaleParam grayscaleParam_;
	SepiaParam sepiaParam_;
	SmoothingParam smoothingParam_;
	GaussianParam gaussianParam_;
	DepthOutlineParam depthOutlineParam_;
	RadialBlurParam radialBlurParam_;
	DissolveParam dissolveParam_;
	RandomParam randomParam_;
	GlitchParam glitchParam_;

	// メタデータマップ
	std::unordered_map<PostEffectType, EffectMeta> effectMetaMap_ = {
	    {PostEffectType::FullScreen,       {L"Fullscreen", 0, nullptr}                                           },
	    {PostEffectType::Vignette,         {L"Vignette", sizeof(VignetteParam), &vignetteParam_}                 },
	    {PostEffectType::Grayscale,        {L"Grayscale", sizeof(GrayscaleParam), &grayscaleParam_}              },
	    {PostEffectType::Sepia,            {L"Sepia", sizeof(SepiaParam), &sepiaParam_}                          },
	    {PostEffectType::Smoothing,        {L"BoxFilter", sizeof(SmoothingParam), &smoothingParam_}              },
	    {PostEffectType::Gaussian,         {L"GaussianFilter", sizeof(GaussianParam), &gaussianParam_}           },
	    {PostEffectType::LuminanceOutline, {L"LuminanceBasedOutline", 0, nullptr}                                },
	    {PostEffectType::DepthOutline,     {L"DepthBasedOutline", sizeof(DepthOutlineParam), &depthOutlineParam_}},
	    {PostEffectType::RadialBlur,       {L"RadialBlur", sizeof(RadialBlurParam), &radialBlurParam_}           },
	    {PostEffectType::Dissolve,         {L"Dissolve", sizeof(DissolveParam), &dissolveParam_}                 },
	    {PostEffectType::Random,           {L"Random", sizeof(RandomParam), &randomParam_}                       },
	    {PostEffectType::Glitch,           {L"Glitch", sizeof(GlitchParam), &glitchParam_}                       },
	};

	// CB
	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;

	// Data
	void* cbData_ = nullptr;

	// TextureManagerポインタ
	TextureManager* textureManager_ = nullptr;
};
} // namespace TinyEngine