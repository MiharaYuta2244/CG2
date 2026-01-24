#include "EnemyHPGauge.h"
#include "SpriteCommon.h"
#include "TextureManager.h"

void EnemyHPGauge::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// HPゲージスプライトの初期化
	spriteHPBar_ = std::make_unique<Sprite>();
	spriteHPBar_->Initialize(ctx_, "resources/white.png");
	spriteHPBar_->SetPosition({340.0f, 668.0f});
	spriteHPBar_->SetSize({spriteHPBarSizeX_, 32.0f});
	spriteHPBar_->SetColor({0.69f, 0.13f, 0.1f, 1.0f});
	spriteHPBar_->SetEnableShine(true);
	spriteHPBar_->SetShineColor({0.9f, 0.1f, 0.1f, 1.0f});
	spriteHPBar_->SetShineParams({0.0f, 0.7f, 0.6f, 1.0f});

	// HPゲージスプライト残像の初期化
	spriteHPBarAfter_ = std::make_unique<Sprite>();
	spriteHPBarAfter_->Initialize(ctx_, "resources/white.png");
	spriteHPBarAfter_->SetPosition({340.0f, 668.0f});
	spriteHPBarAfter_->SetSize({spriteHPBarSizeX_, 32.0f});
	spriteHPBarAfter_->SetColor({0.5f, 0.5f, 0.1f, 1.0f});

	// HPゲージ背景の初期化
	spriteHPBarBG_ = std::make_unique<Sprite>();
	spriteHPBarBG_->Initialize(ctx_, "resources/white.png");
	spriteHPBarBG_->SetPosition({335.0f, 663.0f});
	spriteHPBarBG_->SetSize({610.0f, 42.0f});
	spriteHPBarBG_->SetColor({0.0f, 0.0f, 0.0f, 1.0f});
}

void EnemyHPGauge::Update(float deltaTime) {
	spriteHPBar_->SetSize({spriteHPBarSizeX_, 32.0f});

	// 遅延カウンターがアクティブなら減少
	if (afterImageDelayCounter_ > 0.0f) {
		afterImageDelayCounter_ -= deltaTime;
	} else if (spriteHPBarAfterSizeX_ > spriteHPBarSizeX_) {
		if (!afterAnimation_.anim.GetIsActive()) {
			// 残像アニメーション初期設定
			afterAnimation_.anim = {spriteHPBarAfterSizeX_, spriteHPBarSizeX_, 1.0f, EaseType::EASEINCUBIC};
		} else {
			bool playing = afterAnimation_.anim.Update(deltaTime, afterAnimation_.temp);
			spriteHPBarAfterSizeX_ = afterAnimation_.temp; // サイズの更新

			if (!playing) {
				afterImageDelayCounter_ = kAfterImageDelayFrames; // 遅延時間のリセット
			}
		}
	}

	// サイズの適用
	spriteHPBarAfter_->SetSize({spriteHPBarAfterSizeX_, 32.0f});

	spriteHPBar_->Update();
	spriteHPBarBG_->Update();
	spriteHPBarAfter_->Update();
}

void EnemyHPGauge::Draw() {
	spriteHPBarBG_->Draw();
	spriteHPBarAfter_->Draw();
	spriteHPBar_->Draw();
}

void EnemyHPGauge::HPBarSpriteApply(int hp, int maxHP) {
	// HPが0未満なら早期リターン
	if (hp < 0)
		return;

	// 割合を求める
	float rate = static_cast<float>(hp) / static_cast<float>(maxHP);

	// HP量に応じてスプライトのサイズ変更
	spriteHPBarSizeX_ = kMaxSpriteHPBarSizeX * rate;
}
