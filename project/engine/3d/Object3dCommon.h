#pragma once
#include "DebugCamera.h"
#include "DirectXCommon.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

class Object3dCommon {
public:
	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingCommon();

	void Initialize(DirectXCommon* dxCommon);

	void Update();

	// カメラのセッター
	void SetDefaultCamera(DebugCamera* camera) { defaultCamera_ = camera; }

	// getter
	DirectXCommon* GetDxCommon() const { return dxCommon_; }
	DebugCamera* GetDefaultCamera() { return defaultCamera_; }
	DirectionalLight& GetDirectionalLight() { return globalDirectionalLight_; }
	PointLight& GetPointLight() { return globalPointLight_; }
	SpotLight& GetSpotLight() { return globalSpotLight_; }

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

	void InitializeShaderCompiler();

	// コンパイルシェーダー
	IDxcBlob* CompileShader(
	    // CompilerするShaderファイルへのパス
	    const std::wstring& filePath,
	    // Compilerに使用するProfile
	    const wchar_t* profile,
	    // 初期化で生成したものを3つ
	    IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);

	// グローバルライティングバッファの更新
	void UpdateGlobalLightingBuffers();

	// グローバルライティングリソースの生成
	void CreateGlobalDirectionalLightData();
	void CreateGlobalPointLightData();
	void CreateGlobalSpotLightData();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeBytes);

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
	DebugCamera* defaultCamera_ = nullptr;

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
