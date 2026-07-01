#pragma once
#include "Sprite.h"
#include "AnimationBundle.h"

class PlayerHPIcon {
public:
	void Initialize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

	void DmageAnimStart(int index);

private:
	// HPのUI
	std::array<std::unique_ptr<TinyEngine::Sprite>, 3> sprites_;

	// 座標基準点
	Vector2 basePos_ = {60.0f, 60.0f};

	// X座標間隔
	float margin_ = 70.0f;

	// アニメーション用変数
	std::array<AnimationBundle<float>, 3> scaleYAnims_;
	float baseScaleY_ = 0.0f;
};
