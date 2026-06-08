#include "RetryButton.h"

void RetryButton::Initialize(EngineContext* ctx) {
	IButton::Initialize(ctx);

	// 個別のスプライト設定
	sprite_->SetTexture("Retry.png");
	sprite_->SetPosition({250.0f, 500.0f});
}

void RetryButton::Update(bool isSelect) { IButton::Update(isSelect); }

void RetryButton::Draw() { IButton::Draw(); }

const ButtonStyle& RetryButton::GetNormalStyle() const {
	static ButtonStyle style = {
	    {1, 1, 1, 1}, // 白
	    {206, 80}  // 通常サイズ
	};
	return style;
}

const ButtonStyle& RetryButton::GetSelectedStyle() const {
	static ButtonStyle style = {
	    {1, 1, 0, 1}, // 黄色
	    {206 * 1.2f, 80 * 1.2f}  // 拡大
	};
	return style;
}
