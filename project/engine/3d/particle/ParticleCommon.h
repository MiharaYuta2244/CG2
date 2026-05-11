#pragma once
#include "DebugCamera.h"
#include "DirectXCommon.h"

/// <summary>
/// パーティクルの共通クラス
/// </summary>
class ParticleCommon {
public:
	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingCommon();

	/// <summary>
	/// 初期化関数
	/// </summary>
	/// <param name="dxCommon">DirectX共通クラスのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	// setter
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }

	// getter
	DirectXCommon* GetDxCommon() const { return dxCommon_; }
	Camera* GetDefaultCamera() { return defaultCamera_; }

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

private:
	DirectXCommon* dxCommon_;

	// Root Signature / Pipeline
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// Shader Compiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	// カメラ
	Camera* defaultCamera_ = nullptr;
};
