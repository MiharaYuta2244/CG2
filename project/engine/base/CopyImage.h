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
	LuminanceOutline,
	DepthOutline,
	RadialBlur,
	Dissolve,
	Random,
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
	void Initialize(DirectXCommon* dx, PostEffectType type);

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
	    {PostEffectType::RadialBlur,       {L"RadialBlur", sizeof(RadialBlurParam), &radialBlurParam_}                                   },
	    {PostEffectType::Dissolve,         {L"Dissolve", 0, nullptr}                                             },
	    {PostEffectType::Random,           {L"Random", 0, nullptr}                                               },
	};

	// CB
	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;

	// Data
	void* cbData_ = nullptr;
};
} // namespace TinyEngine