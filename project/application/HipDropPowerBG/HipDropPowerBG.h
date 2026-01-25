#pragma once
#include "AnimationBundle.h"
#include "Sprite.h"
#include "EngineContext.h"
#include <array>
#include <memory>

class HipDropPowerBG {
public:
	void Initialize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

private:
	enum class AnimState {
		Forward, // 通常アニメーション
		Backward // 戻りアニメーション
	};

private:
	// 頂点アニメーションの初期設定
	void AnimationSetting(int index);

	// 頂点戻りアニメーションの初期設定
	void AnimationSettingAfter(int index);

private:
	// プレイヤーの攻撃力背景スプライト
	std::unique_ptr<Sprite> hipDropPowerBGSprite_;

	// スプライト4頂点のアニメーション
	std::array<AnimationBundle<Vector2>, 4> vertexAnimations_;

	// スプライト4頂点の戻りアニメーション
	std::array<AnimationBundle<Vector2>, 4> afterVertexAnimations_;

	// オフセット
	Vector2 offset_[4];

	// アニメーションの状態
	AnimState animState_[4];
};
