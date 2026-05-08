#include "SpriteScaleWipeAnimator.h"

using namespace TinyEngine;

void SpriteScaleWipeAnimator::Initialize(EngineContext* ctx, Vector2 basePos, Vector2 baseSize) {
	basePos_ = basePos;
	baseSize_ = baseSize;

	// アニメーション用単色スプライト生成&初期化
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(ctx, "white.png");
	sprite_->SetAnchorPoint({0.0f, 0.0f});
	sprite_->SetPosition(basePos_);
	sprite_->SetSize(spriteSize_);
}

void SpriteScaleWipeAnimator::Update(float deltaTime) {
#ifdef USE_IMGUI
	ImGui::Begin("ScoreText");

	ImGui::PushID(this);
	ImGui::DragFloat2("Pos", &basePos_.x, 1.0f);
	ImGui::DragFloat2("Size", &baseSize_.x, 1.0f);
	ImGui::PopID();

	ImGui::End();
#endif

	// == アニメーションの継続フラグ ==
	bool wasPlayingBefore = scaleBeforeAnim_.anim.GetIsActive();
	bool playingBefore = scaleBeforeAnim_.anim.Update(deltaTime, scaleBeforeAnim_.temp);
	bool playingAfter = scaleAfterAnim_.anim.Update(deltaTime, scaleAfterAnim_.temp);

	// 行きのアニメーションが終わったら戻りのアニメーションを開始
	if (wasPlayingBefore && !playingBefore) {
		scaleAfterAnim_.anim.Start(baseSize_.x, 0.0f, 0.2f, EaseType::EASEOUTCUBIC);
		isBeforeAnimFinished_ = true;
	}

	// == アニメーションの更新 ==
	if (playingBefore) {
		spriteSize_.x = scaleBeforeAnim_.temp;
		spritePos_.x = basePos_.x;
	} else if (playingAfter) {
		spriteSize_.x = scaleAfterAnim_.temp;
		float rightFixed = basePos_.x + baseSize_.x;
		spritePos_.x = rightFixed - spriteSize_.x;
	}

	// == 座標等の設定&更新 ==
	spritePos_.y = basePos_.y;
	spriteSize_.y = baseSize_.y;
	sprite_->SetPosition(spritePos_);
	sprite_->SetSize(spriteSize_);
	sprite_->Update();
}

void SpriteScaleWipeAnimator::Draw() { sprite_->Draw(); }

void SpriteScaleWipeAnimator::StartAnimation() { scaleBeforeAnim_.anim.Start(0.0f, baseSize_.x, 0.2f, EaseType::EASEOUTCUBIC); }
