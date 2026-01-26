#include "CrossEffect.h"

void CrossEffect::Initialize(EngineContext* ctx, Vector3 targetPos) {
	// 十字エフェクトスプライト
	crossSprite_ = std::make_unique<Sprite>();
	crossSprite_->Initialize(ctx, "resources/Cross.png");
	crossSprite_->SetPosition({0.0f, 0.0f});
	crossSprite_->SetSize({0.0f, 0.0f});
	crossSprite_->SetAnchorPoint({0.5f, 0.5f});
	crossSprite_->SetColor({0.2f, 0.0f, 0.2f, 1.0f});
	crossSprite_->SetEnableShine(true);
	crossSprite_->SetShineParams({0.0f, 0.4f, 0.5f, 1.0f});
	crossSprite_->SetShineColor({0.5f, 0.2f, 0.5f, 1.0f});
	crossSprite_->SetZDepth(100.0f);

	// 十字エフェクトアニメーション
	crossScaleAnimation_.anim = {
	    {0.0f,    0.0f   },
        {1600.0f, 1600.0f},
        1.0f, EaseType::EASEOUTBACK
    };

	crossRotateAnimation_.anim = {0.0f, std::numbers::pi_v<float> * 2, 1.0f, EaseType::EASEOUTCUBIC};

	// 目標ポジション
	targetPos_ = targetPos;

	// 座標変換便利クラス
	screenSpaceUtility_ = std::make_unique<ScreenSpaceUtility>();
}

void CrossEffect::Update(float deltaTime, DebugCamera* camera) {
	// 十字エフェクト
	crossScaleAnimation_.anim.Update(deltaTime, crossScaleAnimation_.temp);
	crossRotateAnimation_.anim.Update(deltaTime, crossRotateAnimation_.temp);

	// カメラのセット
	screenSpaceUtility_->SetCamera(camera);

	crossSprite_->SetSize(crossScaleAnimation_.temp);
	//crossSprite_->SetRotation(crossRotateAnimation_.temp);
	crossSprite_->SetPosition(screenSpaceUtility_->WorldToScreen({targetPos_.x, targetPos_.y, targetPos_.z}, {0.0f, 0.0f}));
	crossSprite_->Update();
}

void CrossEffect::Draw() { crossSprite_->Draw(); }