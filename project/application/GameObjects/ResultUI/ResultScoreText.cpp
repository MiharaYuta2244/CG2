#include "ResultScoreText.h"

using namespace TinyEngine;

void ResultScoreText::Initialize(EngineContext* ctx) {
	animSprite_ = std::make_unique<SpriteScaleWipeAnimator>();
	animSprite_->Initialize(ctx, {300.0f, 400.0f}, {400.0f, 100.0f});
}

void ResultScoreText::Update(float deltaTime, DirectInput* input) {
	// アニメーションの開始処理
	if (input->KeyTriggered(DIK_F1)) {
		animSprite_->StartAnimation();
	}

	// アニメーションスプライトの更新
	animSprite_->Update(deltaTime);
}

void ResultScoreText::Draw() { animSprite_->Draw(); }