#include "TestScene.h"
#include "LevelDataLoader.h"
#include "SceneManager.h"

using namespace TinyEngine;

void TestScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;

	gpuParticle_ = std::make_unique<GPUParticle>();
	gpuParticle_->Initialize(ctx.engineContext, "white.png");
	gpuParticle_->DispatchInitialize();
}

void TestScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

	perView_ = {ctx_.currentCamera->GetViewProjectionMatrix(), MathUtility::MakeIdentity4x4()};
	gpuParticle_->Update(perView_, deltaTime);
	gpuParticle_->DispatchEmit();
}

void TestScene::Draw() {
	gpuParticle_->Draw();
}

void TestScene::Finalize() {}