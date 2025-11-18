#pragma once
#include <memory>
#include "Sprite.h"

class SpriteCommon;
class TextureManager;

class EnemyHPGauge {
public:
	void Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager);

	void Update();

	void Draw();

	// HP量をスプライトのサイズに適用
	void HPBarSpriteApply(int currentHP, int maxHP);

private:
	// 仮のHPバー
	std::unique_ptr<Sprite> spriteHPBar_ = std::make_unique<Sprite>();
	std::unique_ptr<Sprite> spriteHPBarBG_ = std::make_unique<Sprite>();

	// HPバーのサイズX
	const float kMaxSpriteHPBarSizeX = 1000.0f;
	float spriteHPBarSizeX_ = kMaxSpriteHPBarSizeX;
};
