#include "ToTitleButton.h"

void ToTitleButton::Initialize(EngineContext* ctx) {
	IButton::Initialize(ctx);

	// 個別のスプライト設定
	sprite_->SetTexture("Title.png");
	sprite_->SetPosition({850.0f, 500.0f});
}

void ToTitleButton::Update(bool isSelect) { IButton::Update(isSelect); }

void ToTitleButton::Draw() { IButton::Draw(); }

const ButtonStyle& ToTitleButton::GetNormalStyle() const {
	static ButtonStyle style = {
	    {1, 1, 1, 1}, // 白
	    {176, 80}  // 通常サイズ
	};
	return style;
}

const ButtonStyle& ToTitleButton::GetSelectedStyle() const {
	static ButtonStyle style = {
	    {1, 1, 0, 1}, // 黄色
	    {176 * 1.2f, 80 * 1.2f}  // 拡大
	};
	return style;
}