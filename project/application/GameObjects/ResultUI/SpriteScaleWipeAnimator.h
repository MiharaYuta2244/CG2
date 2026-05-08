#pragma once
#include "AnimationBundle.h"
#include "Sprite.h"

class SpriteScaleWipeAnimator {
public:
	// 初期化
	void Initialize(EngineContext* ctx, Vector2 basePos, Vector2 baseSize);

	// 更新
	void Update(float deltaTime);

	// 描画
	void Draw();

	// アニメーション開始
	void StartAnimation();

private:
	// スプライト
	std::unique_ptr<TinyEngine::Sprite> sprite_;

	// 基本の座標
	Vector2 basePos_ = {0.0f, 0.0f};

	// 基本のサイズ
	Vector2 baseSize_ = {500.0f, 100.0f};

	// スプライトの実際の座標
	Vector2 spritePos_ = basePos_;

	// スプライトの実際のサイズ
	Vector2 spriteSize_ = {0.0f, baseSize_.y};

	// == アニメーション関連 ==
	AnimationBundle<float> scaleBeforeAnim_; // 行き
	AnimationBundle<float> scaleAfterAnim_;  // 戻り
};
