#pragma once
#include "DebugCamera.h"
#include "DirectXCommon.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

/// <summary>
/// 3Dオブジェクトの共通クラス
/// </summary>
class Object3dCommon {
public:
	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingCommon();

	/// <summary>
	/// アウトライン用の設定を適用するメソッド
	/// </summary>
	void DrawSettingOutline();

	/// <summary>
	/// 初期化関数
	/// </summary>
	/// <param name="dxCommon">DirectX共通クラスのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新関数
	/// </summary>
	void Update();

	// Setter
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }
	void SetDirectionalLightIntensity(const float& intensity) { globalDirectionalLight_.intensity = intensity; }

	// getter
	DirectXCommon* GetDxCommon() const { return dxCommon_; }
	Camera* GetDefaultCamera() { return defaultCamera_; }
	DirectionalLight& GetDirectionalLight() { return globalDirectionalLight_; }
	PointLight& GetPointLight() { return globalPointLight_; }
	SpotLight& GetSpotLight() { return globalSpotLight_; }
	ID3D12PipelineState* GetOutlinePipelineState() { return outlinePipelineState_.Get(); }

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
