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

	// 基準になるスケールの記録
	baseScaleY_ = sprites_[0]->GetSize().y;
}

void PlayerHPIcon::Update(float deltaTime) {
	for (int i = 0; i < sprites_.size(); ++i) {
		float margin = i * margin_;
		sprites_[i]->SetPosition({basePos_.x + margin, basePos_.y});

		// アニメーションの更新・適用を各アイコンごとに独立して行う
		bool playing = scaleYAnims_[i].anim.Update(deltaTime, scaleYAnims_[i].temp);

		if (playing) {
			sprites_[i]->SetSize({sprites_[i]->GetSize().x, scaleYAnims_[i].temp});
		}
	}

	for (auto& sprite : sprites_) {
		sprite->Update();
	}

#ifdef USE_IMGUI
	ImGui::Begin("HPIcon");
	ImGui::DragFloat2("BasePos", &basePos_.x, 1.0f);
	ImGui::End();
#endif
}

void PlayerHPIcon::Draw() {
	for (auto& sprite : sprites_) {
		sprite->Draw();
	}
}

void PlayerHPIcon::DmageAnimStart(int index) {
	if (index >= 0 && index < scaleYAnims_.size()) {
		scaleYAnims_[index].anim.Start(baseScaleY_, 0.0f, 0.3f, EaseType::EASEINOUTBACK);
	}
}