#include "HipDropPowerBG.h"

void HipDropPowerBG::Initialize(EngineContext* ctx) {
	hipDropPowerBGSprite_ = std::make_unique<Sprite>();
	hipDropPowerBGSprite_->Initialize(ctx, "resources/white.png");
	hipDropPowerBGSprite_->SetSize({64.0f, 64.0f});
	hipDropPowerBGSprite_->SetPosition({1170.0f, 96.0f});
	hipDropPowerBGSprite_->SetColor({0.8f, 0.1f, 0.1f, 1.0f});
	hipDropPowerBGSprite_->SetShineColor({0.9f, 0.1f, 0.1f, 1.0f});
	hipDropPowerBGSprite_->SetEnableShine(true);
	hipDropPowerBGSprite_->SetShineParams({0.0f, 0.5f, 0.5f, 1.0f});

	// 頂点アニメーションの初期設定
	for (int i = 0; i < 4; ++i) {
		AnimationSetting(i);
		animState_[i] = AnimState::Forward;
	}
}

void HipDropPowerBG::Update(float deltaTime) {
	for (int i = 0; i < 4; ++i) {

		bool playing = false;

		if (animState_[i] == AnimState::Forward) {
			playing = vertexAnimations_[i].anim.Update(deltaTime, offset_[i]);

			if (playing) {
				hipDropPowerBGSprite_->SetVertexOffset(i, offset_[i]);
			} else {
				// Forwardが終わったらBackwardを開始
				AnimationSettingAfter(i);
				animState_[i] = AnimState::Backward;
			}
		} else { // Backward
			playing = afterVertexAnimations_[i].anim.Update(deltaTime, offset_[i]);

			if (playing) {
				hipDropPowerBGSprite_->SetVertexOffset(i, offset_[i]);
			} else {
				// Backwardが終わったらForwardを開始
				AnimationSetting(i);
				animState_[i] = AnimState::Forward;
			}
		}
	}

	hipDropPowerBGSprite_->Update();
}

void HipDropPowerBG::Draw() { hipDropPowerBGSprite_->Draw(); }

void HipDropPowerBG::AnimationSetting(int index) {
	switch (index) {
	case 0:
		vertexAnimations_[0].anim = {
		    offset_[0], {-0.2f, 0.2f},
             0.5f, EaseType::EASEOUTCUBIC
        };
		break;

	case 1:
		vertexAnimations_[1].anim = {
		    offset_[1], {-0.2f, -0.2f},
             0.6f, EaseType::EASEOUTCUBIC
        };
		break;

	case 2:
		vertexAnimations_[2].anim = {
		    offset_[2], {0.2f, 0.2f},
             0.4f, EaseType::EASEOUTCUBIC
        };
		break;

	case 3:
		vertexAnimations_[3].anim = {
		    offset_[3], {0.2f, -0.2f},
             0.3f, EaseType::EASEOUTCUBIC
        };
		break;
	}
}

void HipDropPowerBG::AnimationSettingAfter(int index) {
	switch (index) {
	case 0:
		afterVertexAnimations_[0].anim = {
		    offset_[0], {0.0f, 0.0f},
             0.3f, EaseType::EASEOUTCUBIC
        };
		break;

	case 1:
		afterVertexAnimations_[1].anim = {
		    offset_[1], {0.0f, 0.0f},
             0.5f, EaseType::EASEOUTCUBIC
        };
		break;

	case 2:
		afterVertexAnimations_[2].anim = {
		    offset_[2], {0.0f, 0.0f},
             0.4f, EaseType::EASEOUTCUBIC
        };
		break;

	case 3:
		afterVertexAnimations_[3].anim = {
		    offset_[3], {0.0f, 0.0f},
             0.6f, EaseType::EASEOUTCUBIC
        };
		break;
	}
}
