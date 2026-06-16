#pragma once
#include "CopyImage.h"
#include "RenderTexture.h"

struct PostEffectPass {
	PostEffectType type;
	std::unique_ptr<TinyEngine::CopyImage> copyImage;
	std::unique_ptr<RenderTexture> renderTexture;
};