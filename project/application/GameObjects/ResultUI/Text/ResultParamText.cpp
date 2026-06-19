#include "ResultParamText.h"

using namespace TinyEngine;

void ResultParamText::Initialize(EngineContext* ctx, Vector2 basePos, Vector2 baseSize) {
	// テキスト生成&初期化
	text_ = std::make_unique<Sprite>();
	text_->Initialize(ctx, "Title_Play.png");

	// アニメーションスプライト生成&初期化
	animSprite_ = std::make_unique<SpriteScaleWipeAnimator>();
	animSprite_->Initialize(ctx, basePos, baseSize);
}

void ResultParamText::Update(float deltaTime, DirectInput* input) {
	// アニメーションスプライトの更新
	animSprite_->Update(deltaTime);

	// テキストの更新
	text_->SetPosition(animSprite_->GetPos());
	text_->Update();
}

void ResultParamText::Draw() {
	// テキストの描画
	if (animSprite_->GetBeforeAnimationFinished()) {
		text_->Draw();
	}

	// アニメーションスプライトの描画
	animSprite_->Draw();
}