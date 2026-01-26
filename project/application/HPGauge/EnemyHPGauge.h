#pragma once
#include "AnimationBundle.h"
#include "EngineContext.h"
#include "HPGauge/IHPGauge.h"
#include "Sprite.h"
#include <memory>

class EnemyHPGauge : public IHPGauge {
public:
	void Initialize(EngineContext* ctx) override;

	void Update(float deltaTime) override;

	void Draw() override;

	// HP量をスプライトのサイズに適用
	void HPBarSpriteApply(int currentHP, int maxHP) override;

private:
	// HPの残像アニメーション
	void AnimationHPGauge(float deltaTime) override;
};
