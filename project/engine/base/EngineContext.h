#pragma once
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"
#include "PostEffectPipeline.h"
#include "SkyBoxCommon.h"
#include "SpriteCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"

/// <summary>
/// エンジンのコンテキストを保持する構造体
/// </summary>
struct EngineContext {
	Object3dCommon* object3dCommon = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	TextureManager* textureManager = nullptr;
	ModelManager* modelManager = nullptr;
	ParticleCommon* particleCommon = nullptr;
	SrvManager* srvManager = nullptr;
	SkyboxCommon* skyboxCommon = nullptr;
	PostEffectPipeline* postEffectPipeline = nullptr;
};