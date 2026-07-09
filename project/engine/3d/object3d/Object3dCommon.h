#pragma once
#include "DebugCamera.h"
#include "DirectXCommon.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "TextureManager.h"
#include <d3d12.h>
#include <wrl/client.h>

/// <summary>
/// 3Dオブジェクトの共通クラス
/// </summary>
class Object3dCommon {
public:
	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingCommon(TextureManager* textureManager);

	/// <summary>
	/// アウトライン用の設定を適用する関数
	/// </summary>
	void DrawSettingOutline();

	/// <summary>
	/// スキニング用の設定を適用する関数
	/// </summary>
	void SetSkinningComputePipeline();

	/// <summary>
	/// 半透明用の設定を適用する関数
	/// </summary>
	/// <param name="textureManager"></param>
	void DrawSettingTransparent(TextureManager* textureManager);

	/// <summary>
	/// 初期化関数
	/// </summary>
	/// <param name="dxCommon">DirectX共通クラスのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新関数
	/// </summary>
	void Update();

	// スキニング用パイプラインの生成関数
	void CreateSkinningGraphicsPipeline();

	// 半透明用パイプラインの生成関数
	void CreateTransparentGraphicsPipeline();

	// スキニング用パイプラインの生成関数
	void CreateSkinningComputePipeline();

	// Setter
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }
	void SetDirectionalLightParam(DirectionalLight dirLight) { globalDirectionalLight_ = dirLight; }
	void SetPointLightParam(PointLight pointLight) { globalPointLight_ = pointLight; }
	void SetSpotLightParam(SpotLight spotLight) { globalSpotLight_ = spotLight; }

	// getter
	DirectXCommon* GetDxCommon() const { return dxCommon_; }
	Camera* GetDefaultCamera() { return defaultCamera_; }
	DirectionalLight& GetDirectionalLight() { return globalDirectionalLight_; }
	PointLight& GetPointLight() { return globalPointLight_; }
	SpotLight& GetSpotLight() { return globalSpotLight_; }
	ID3D12PipelineState* GetOutlinePipelineState() { return outlinePipelineState_.Get(); }
	ID3D12RootSignature* GetSkinningRootSignature() const { return skinningRootSignature_.Get(); }
	ID3D12PipelineState* GetSkinningGraphicsPipelineState() const { return skinningGraphicsPipelineState_.Get(); }
	ID3D12PipelineState* GetTransparentGraphicsPipelineState() const { return transparentPipelineState_.Get(); }
	ID3D12RootSignature* GetSkinningComputeRootSignature() const { return skinningComputeRootSignature_.Get(); }
	ID3D12PipelineState* GetSkinningComputePipelineState() const { return skinningComputePipelineState_.Get(); }

#ifdef USE_IMGUI
	// Lighting用ImGuiの描画
	void DrawImGuiLighting();
#endif // USE_IMGUI

private:
	/// <summary>
	/// ルートシグネチャの作成
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// グラフィックスパイプラインの生成
	/// </summary>
	void CreateGraphicsPipeline();

	/// <summary>
	/// シェーダコンパイラの初期化
	/// </summary>
	void InitializeShaderCompiler();

	// グローバルライティングバッファの更新
	void UpdateGlobalLightingBuffers();

	// グローバルライティングリソースの生成
	void CreateGlobalDirectionalLightData();
	void CreateGlobalPointLightData();
	void CreateGlobalSpotLightData();

	// アウトライン用PSO作成メソッド
	void CreateOutlinePipeline();

private:
	DirectXCommon* dxCommon_;

	// Root Signature / Pipeline
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> outlinePipelineState_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> skinningRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> skinningComputeRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skinningGraphicsPipelineState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skinningComputePipelineState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> transparentPipelineState_;

	// Shader Compiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	// カメラ
	Camera* defaultCamera_ = nullptr;

	// グローバルライティングリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> globalDirectionalLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> globalPointLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> globalSpotLightResource_;

	// グローバルライティングデータポインタ
	DirectionalLight* globalDirectionalLightData_ = nullptr;
	PointLight* globalPointLightData_ = nullptr;
	SpotLight* globalSpotLightData_ = nullptr;

	// グローバルライティングデータ
	DirectionalLight globalDirectionalLight_;
	PointLight globalPointLight_;
	SpotLight globalSpotLight_;
};
