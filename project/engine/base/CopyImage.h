#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <dxcapi.h>
#include <wrl.h>

namespace TinyEngine {
class CopyImage {
public:
	// 初期化
	void Initialize(DirectXCommon* dx);

	// 描画
	void Draw(DirectXCommon* dx, SrvManager* srv, uint32_t srvIndex);

private:
	// シェーダコンパイラの初期化
	void InitializeShaderCompiler();

	// ルートシグネチャ作成
	void CreateRootSignature();

	// グラフィックスパイプライン作成
	void CreateGraphicsPipeline(DirectXCommon* dx);

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

	// Shader compiler objects
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};
} // namespace TinyEngine