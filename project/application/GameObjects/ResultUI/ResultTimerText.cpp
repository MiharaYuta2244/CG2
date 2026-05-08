#include "ResultTimerText.h"

using namespace TinyEngine;

void ResultTimerText::Initialize(EngineContext* ctx) {
	// アニメーションスプライトの生成&初期化
	animSprite_ = std::make_unique<SpriteScaleWipeAnimator>();
	animSprite_->Initialize(ctx, {300.0f, 200.0f}, {400.0f, 100.0f});
}

void ResultTimerText::Update(float deltaTime, DirectInput* input) {
	// アニメーションの開始処理
	if (input->KeyTriggered(DIK_F1)) {
		animSprite_->StartAnimation();
	}

	// アニメーションスプライトの更新
	animSprite_->Update(deltaTime);
}

void ResultTimerText::Draw() { animSprite_->Draw(); }