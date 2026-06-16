#include "PostEffectPipeline.h"

using namespace TinyEngine;

void PostEffectPipeline::Inititlize(DirectXCommon* dxCommon, SrvManager* srvManager, TextureManager* textureManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	textureManager_ = textureManager;
}

void PostEffectPipeline::SetEffects(const std::vector<PostEffectType>& types) {
	passes_.clear();

	for (size_t i = 0; i < types.size(); ++i) {
		PostEffectPass pass;
		pass.type = types[i];

		// CopyImage生成
		pass.copyImage = std::make_unique<TinyEngine::CopyImage>();
		pass.copyImage->Initialize(dxCommon_, types[i], textureManager_);

		if (i < types.size() - 1) {
			pass.renderTexture = std::make_unique<RenderTexture>();
			pass.renderTexture->Initialize(dxCommon_, srvManager_, WinApp::kClientWidth, WinApp::kClientHeight);
		}

		passes_.push_back(std::move(pass));
	}
}

void PostEffectPipeline::Excute(DirectXCommon* dxCommon, SrvManager* srvManager, uint32_t inputColorSrv, uint32_t inputDepthSrv) {
	uint32_t currentColorSrv = inputColorSrv;
	uint32_t currentDepthSrv = inputDepthSrv;

	for (size_t i = 0; i < passes_.size(); ++i) {
		auto& pass = passes_[i];
		bool isLast = (i == passes_.size() - 1);

		if (!isLast) {
			// 中間バッファへ書き込み開始
			pass.renderTexture->BeginRender(dxCommon_);
		} else {
			// 最後のパスはスワップチェーンのバックバッファをRTとして復元する
			auto cmd = dxCommon_->GetCommandList();
			D3D12_CPU_DESCRIPTOR_HANDLE backBufferRTV = dxCommon_->GetCurrentBackBufferRTV();
			cmd->OMSetRenderTargets(1, &backBufferRTV, false, nullptr);

			D3D12_VIEWPORT vp = dxCommon_->CreateViewport();
			D3D12_RECT sc = dxCommon_->CreateScissor();
			cmd->RSSetViewports(1, &vp);
			cmd->RSSetScissorRects(1, &sc);
		}

		pass.copyImage->Draw(dxCommon_, srvManager_, currentColorSrv, currentDepthSrv);

		if (!isLast) {
			pass.renderTexture->EndRender(dxCommon_);
			// 次のパスへ中間バッファを渡す
			currentColorSrv = pass.renderTexture->GetSRVIndexColor();
			currentDepthSrv = pass.renderTexture->GetSRVIndexDepth();
		}
	}
}

TinyEngine::CopyImage* PostEffectPipeline::GetPass(PostEffectType type) {
	for (auto& pass : passes_) {
		if (pass.type == type) {
			return pass.copyImage.get();
		}
	}
	return nullptr;
}