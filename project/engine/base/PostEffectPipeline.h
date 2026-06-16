#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "PostEffectPass.h"
#include "CopyImage.h"

class PostEffectPipeline {
public:
	void Inititlize(DirectXCommon* dxCommon, SrvManager* srvManager, TextureManager* textureManager);

	void SetEffects(const std::vector<PostEffectType>& types);

	void Excute(DirectXCommon* dxCommon, SrvManager* srvManager, uint32_t inputColorSrv, uint32_t inputDepthSrv);

	TinyEngine::CopyImage* GetPass(PostEffectType type);

private:
	std::vector<PostEffectPass> passes_;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
};
