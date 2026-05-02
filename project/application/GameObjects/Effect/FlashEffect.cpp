#include "FlashEffect.h"

using namespace TinyEngine;

void FlashEffect::Initialize(EngineContext* ctx) {
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(ctx, "white.png");
	sprite_->SetSize({1280.0f, 720.0f});
}

void FlashEffect::Update(float deltaTime) {
	if (!isActive_)
		return;

	// フラッシュ減衰
	if (flashIntensity_ > 0.0f) {
		flashIntensity_ -= deltaTime;
		flashIntensity_ = std::max(flashIntensity_, 0.0f);

		if (flashIntensity_ <= 0.0f) {
			isFinished_ = true;
		}
	}

	Vector4 color = {1.0f, 1.0f, 1.0f, flashIntensity_};
	sprite_->SetColor(color);
	sprite_->Update();
}

void FlashEffect::Draw() {
	if (!isActive_)
		return;

	sprite_->Draw();
}

void FlashEffect::Trigger() {
	if (isActive_)
		return;

	isActive_ = true;
}
