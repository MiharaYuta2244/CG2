#pragma once
#include "AnimationBundle.h"
#include "EngineContext.h"
#include "HPGauge/IHPGauge.h"
#include "Sprite.h"
#include <memory>

class PlayerHPGauge : public IHPGauge {
public:
	void Initialize(EngineContext* ctx) override;

	void Update(float deltaTime) override;

	void Draw() override;

	// HP量をスプライトのサイズに適用
	void HPBarSpriteApply(int currentHP, int maxHP) override;

	// setter
	void SetPosition(const Vector2& position) {
		spriteHPBar_->SetPosition(position);
		spriteHPBarAfter_->SetPosition(position);
		spriteHPBarBG_->SetPosition({position.x - 5.0f, position.y - 5.0f});
	}

private:
	// HPの残像アニメーション
	void AnimationHPGauge(float deltaTime) override;
};
