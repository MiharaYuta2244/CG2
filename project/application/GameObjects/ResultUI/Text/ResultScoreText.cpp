#include "ResultScoreText.h"

using namespace TinyEngine;

void ResultScoreText::Initialize(EngineContext* ctx) {
	// テキスト生成&初期化
	text_ = std::make_unique<Sprite>();
	text_->Initialize(ctx, "Title_Play.png");
	text_->SetPosition(basePos_);

	// アニメーションスプライト生成&初期化
	animSprite_ = std::make_unique<SpriteScaleWipeAnimator>();
	animSprite_->Initialize(ctx, basePos_, baseSize_);
}

void ResultScoreText::Update(float deltaTime, DirectInput* input) {
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

void ResultScoreText::Draw() {
	// テキストの描画
	if (animSprite_->GetBeforeAnimationFinished()) {
		text_->Draw();
	}

	// アニメーションスプライトの描画
	animSprite_->Draw();
}