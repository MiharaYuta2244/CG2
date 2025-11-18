#include "EnemyHPGauge.h"
#include "SpriteCommon.h"
#include "TextureManager.h"

void EnemyHPGauge::Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager) {
	spriteHPBar_->Initialize(spriteCommon, textureManager, "resources/white.png");
	spriteHPBarBG_->Initialize(spriteCommon, textureManager, "resources/white.png");

	// HPゲージスプライトの初期化
	spriteHPBar_->SetPosition({140.0f, 668.0f});
	spriteHPBarBG_->SetPosition({135.0f, 663.0f});

	spriteHPBar_->SetSize({spriteHPBarSizeX_, 32.0f});
	spriteHPBarBG_->SetSize({1010.0f, 42.0f});

	spriteHPBar_->SetColor({0.69f, 0.13f, 0.1f, 1.0f});
	spriteHPBarBG_->SetColor({0.0f, 0.0f, 0.0f, 1.0f});
}

void EnemyHPGauge::Update() {
	spriteHPBar_->SetSize({spriteHPBarSizeX_, 32.0f});

	spriteHPBar_->Update();
	spriteHPBarBG_->Update();
}

void EnemyHPGauge::Draw() {
	spriteHPBarBG_->Draw();
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
