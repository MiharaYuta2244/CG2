#include "IButton.h"
#include "MathOperator.h"

using namespace TinyEngine;

void IButton::Initialize(EngineContext* ctx) {
	// スプライトの生成&初期化
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(ctx, "Title_Play.png");
}

void IButton::Update(bool isSelect) {
	// 選択状況を受け取る
	isSelect_ = isSelect;

	// 状態に応じたパラメータの設定
	const ButtonStyle& style = isSelect_ ? GetSelectedStyle() : GetNormalStyle();
	sprite_->SetColor(style.color);
	sprite_->SetSize(style.size);

	// 更新
	sprite_->Update();
}

void IButton::Draw() {
	// スプライトの描画
	sprite_->Draw();
}

void IButton::SetCallback(std::function<void()> callback) { callback_ = std::move(callback); }

void IButton::Execute() {
	if (callback_)
		callback_();
}
