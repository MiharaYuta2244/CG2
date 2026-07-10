#pragma once
#include "AnimationBundle.h"
#include "Sprite.h"

class PlayerHPIcon {
public:
	void Initialize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

	// 被ダメージ時のアイコンのアニメーション開始処理
	void DmageAnimStart(int index);

	// 回復時のアイコンのアニメーション開始処理
	void HealAnimStart(int index);

private:
	// HPのUI
	std::array<std::unique_ptr<TinyEngine::Sprite>, 3> sprites_;

	// Iconの背景
	std::unique_ptr<TinyEngine::Sprite> bgSprite_;

	// 座標基準点
	Vector2 basePos_ = {60.0f, 60.0f};

	// 背景スプライト座標
	Vector2 bgPos_ = {0.0f, 65.0f};

	// 背景スプライトサイズ
	Vector2 bgSize_ = {280.0f, 30.0f};

	// 背景スプライト色
	Vector4 bgColor_ = {1.0f, 0.0f, 0.1f, 0.3f};

	// X座標間隔
	float margin_ = 70.0f;

	// アニメーション用変数
	std::array<AnimationBundle<float>, 3> scaleYAnims_;
	float baseScaleY_ = 0.0f;
};
