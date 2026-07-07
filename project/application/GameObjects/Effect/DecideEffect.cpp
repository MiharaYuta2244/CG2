#include "DecideEffect.h"

using namespace TinyEngine;

void DecideEffect::Initialize(EngineContext* ctx, Vector2 pos) {
	// 1枚目
	spriteFirst_ = std::make_unique<Sprite>();
	spriteFirst_->Initialize(ctx, "white.png");
	spriteFirst_->SetSize(startSize_);
	spriteFirst_->SetPosition(pos + (startSize_ / 2.0f));
	spriteFirst_->SetAnchorPoint({0.5f, 0.5f});
	spriteFirst_->SetColor(spriteFirstColor_);

	// 1枚目
	spriteSecond_ = std::make_unique<Sprite>();
	spriteSecond_->Initialize(ctx, "white.png");
	spriteSecond_->SetSize(startSize_);
	spriteSecond_->SetPosition(pos + (startSize_ / 2.0f));
	spriteSecond_->SetAnchorPoint({0.5f, 0.5f});
	spriteSecond_->SetColor(spriteSecondColor_);
}

void DecideEffect::Update(float deltaTime) {
	// 1枚目のアニメーション更新
	bool playingWidthAnim = widthAnim_.anim.Update(deltaTime, widthAnim_.temp);
	bool playingHeightAnim = heightAnim_.anim.Update(deltaTime, heightAnim_.temp);

	if (playingWidthAnim && playingHeightAnim) {
		spriteFirst_->SetSize({widthAnim_.temp, heightAnim_.temp});
	}

	// 2枚目のアニメーション更新
	bool playingSecondWidth = secondWidthAnim_.anim.Update(deltaTime, secondWidthAnim_.temp);
	bool playingSecondHeight = secondHeightAnim_.anim.Update(deltaTime, secondHeightAnim_.temp);
	bool playingSecondAlpha = secondAlphaAnim_.anim.Update(deltaTime, secondAlphaAnim_.temp);

	// 2枚目のサイズ適用
	if (playingSecondWidth && playingSecondHeight) {
		spriteSecond_->SetSize({secondWidthAnim_.temp, secondHeightAnim_.temp});
	}

	// 2枚目の透明度適用
	if (playingSecondAlpha) {
		spriteSecondColor_.w = secondAlphaAnim_.temp;
		spriteSecond_->SetColor(spriteSecondColor_);
	}

	spriteFirst_->Update();
	spriteSecond_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("DecideEffect");
	ImGui::ColorEdit4("First Color", &spriteFirstColor_.x);
	ImGui::ColorEdit4("Second Color", &spriteSecondColor_.x);
	ImGui::End();
	spriteFirst_->SetColor(spriteFirstColor_);
#endif
}

void DecideEffect::Draw() {
	if (isStartAnimation_) {
		spriteFirst_->Draw();
		spriteSecond_->Draw();
	}
}

void DecideEffect::StartAnimation() {
	if (isStartAnimation_) {
		return;
	}

	// アニメーション開始フラグを立てる
	isStartAnimation_ = true;

	// 1枚目
	widthAnim_.anim.Start(startSize_.x, endSize_.x, 0.2f, EaseType::EASEOUTCUBIC);
	heightAnim_.anim.Start(startSize_.y, endSize_.y, 0.2f, EaseType::EASEOUTCUBIC);

	// 2枚目
	secondWidthAnim_.anim.Start(startSize_.x, secondEndSize_.x, 0.3f, EaseType::EASEOUTCUBIC);
	secondHeightAnim_.anim.Start(startSize_.y, secondEndSize_.y, 0.3f, EaseType::EASEOUTCUBIC);
	secondAlphaAnim_.anim.Start(1.0f, 0.0f, 0.3f, EaseType::EASEOUTCUBIC);
}

void DecideEffect::SetPos(Vector2 pos) {
	Vector2 finalPos = pos + (startSize_ / 2.0f);
	spriteFirst_->SetPosition(finalPos);
	spriteSecond_->SetPosition(finalPos);
}