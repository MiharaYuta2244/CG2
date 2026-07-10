#include "PlayerHPIcon.h"

using namespace TinyEngine;

void PlayerHPIcon::Initialize(EngineContext* ctx) {
	for (int i = 0; i < sprites_.size(); ++i) {
		sprites_[i] = std::make_unique<Sprite>();
		sprites_[i]->Initialize(ctx, "GorillaIcon.png");
		float margin = i * margin_;
		sprites_[i]->SetPosition({basePos_.x + margin, basePos_.y});
		sprites_[i]->SetAnchorPoint({0.5f, 0.5f});
	}

	// 背景スプライト生成&初期化
	bgSprite_ = std::make_unique<Sprite>();
	bgSprite_->Initialize(ctx, "white.png");
	bgSprite_->SetPosition(bgPos_);
	bgSprite_->SetSize(bgSize_);
	bgSprite_->SetColor(bgColor_);

	// 基準になるスケールの記録
	baseScaleY_ = sprites_[0]->GetSize().y;
}

void PlayerHPIcon::Update(float deltaTime) {
	for (int i = 0; i < sprites_.size(); ++i) {
		float margin = i * margin_;
		sprites_[i]->SetPosition({basePos_.x + margin, basePos_.y});

		// アニメーションの更新・適用を各アイコンごとに行う
		bool playing = scaleYAnims_[i].anim.Update(deltaTime, scaleYAnims_[i].temp);

		if (playing) {
			sprites_[i]->SetSize({sprites_[i]->GetSize().x, scaleYAnims_[i].temp});
		}
	}

	for (auto& sprite : sprites_) {
		sprite->Update();
	}

	// 背景スプライト更新
	bgSprite_->Update();
}

void PlayerHPIcon::Draw() {
	// 背景スプライト描画
	bgSprite_->Draw();

	for (auto& sprite : sprites_) {
		sprite->Draw();
	}
}

void PlayerHPIcon::DmageAnimStart(int index) {
	if (index >= 0 && index < scaleYAnims_.size()) {
		scaleYAnims_[index].anim.Start(baseScaleY_, 0.0f, 0.3f, EaseType::EASEINOUTBACK);
	}
}

void PlayerHPIcon::HealAnimStart(int index) {
	if (index >= 0 && index < scaleYAnims_.size()) {
		scaleYAnims_[index].anim.Start(0.0f, baseScaleY_, 0.3f, EaseType::EASEINOUTBACK);
	}
}