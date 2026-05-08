#include "ResultTimerText.h"

using namespace TinyEngine;

void ResultTimerText::Initialize(EngineContext* ctx) {
	// テキスト生成&初期化
	text_ = std::make_unique<Sprite>();
	text_->Initialize(ctx, "Title_Play.png");
	text_->SetPosition(basePos_);

	// アニメーションスプライトの生成&初期化
	animSprite_ = std::make_unique<SpriteScaleWipeAnimator>();
	animSprite_->Initialize(ctx, basePos_, baseSize_);
}

void ResultTimerText::Update(float deltaTime, DirectInput* input) {
	// アニメーションの開始処理
	if (input->KeyTriggered(DIK_F1)) {
		animSprite_->StartAnimation();
	}

	// アニメーションスプライトの更新
	animSprite_->Update(deltaTime);

	// テキストの更新
	text_->SetPosition(animSprite_->GetPos());
	text_->Update();
}

void ResultTimerText::Draw() {
	// テキストの描画
	if (animSprite_->GetBeforeAnimationFinished()) {
		text_->Draw();
	}

	// アニメーションスプライトの描画
	animSprite_->Draw();
}