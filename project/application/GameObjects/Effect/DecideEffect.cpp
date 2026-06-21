#include "DecideEffect.h"

using namespace TinyEngine;

void DecideEffect::Initialize(EngineContext* ctx, Vector2 pos) {
	// 一枚目
	spriteFirst_ = std::make_unique<Sprite>();
	spriteFirst_->Initialize(ctx, "white.png");
	spriteFirst_->SetSize(startSize_);
	spriteFirst_->SetPosition(pos + (startSize_ / 2.0f));
	spriteFirst_->SetAnchorPoint({0.5f, 0.5f});
	spriteFirst_->SetColor(spriteFirstColor_);

	// 二枚目
	spriteSecond_ = std::make_unique<Sprite>();
	spriteSecond_->Initialize(ctx, "white.png");
	spriteSecond_->SetSize(startSize_);
	spriteSecond_->SetPosition(pos);
}

void DecideEffect::Update(float deltaTime) {
	bool playingWidthAnim = widthAnim_.anim.Update(deltaTime, widthAnim_.temp);
	bool playingHeightAnim = heightAnim_.anim.Update(deltaTime, heightAnim_.temp);

	if (playingWidthAnim && playingHeightAnim) {
		spriteFirst_->SetSize({widthAnim_.temp, heightAnim_.temp});
	}

	spriteFirst_->Update();
	spriteSecond_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("DecideEffect");
	ImGui::ColorEdit4("Color", &spriteFirstColor_.x);
	ImGui::End();
	spriteFirst_->SetColor(spriteFirstColor_);
#endif
}

void DecideEffect::Draw() {
	if (isStartAnimation_) {
		spriteFirst_->Draw();
	}

	//spriteSecond_->Draw();
}

void DecideEffect::StartAnimation() {
	if (isStartAnimation_) {
		return;
	}

	// アニメーション開始フラグを立てる
	isStartAnimation_ = true;

	widthAnim_.anim.Start(startSize_.x, endSize_.x, 0.2f, EaseType::EASEOUTCUBIC);
	heightAnim_.anim.Start(startSize_.y, endSize_.y, 0.2f, EaseType::EASEOUTCUBIC);
}