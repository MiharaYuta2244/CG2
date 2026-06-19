#pragma once
#include "AnimationBundle.h"
#include "GameObjects/ResultUI/Animation/SpriteScaleWipeAnimator.h"
#include "Sprite.h"
#include <variant>

/// <summary>
/// リザルトシーンで倒した敵の数を表すテキスト
/// </summary>
class ResultParamText {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, Vector2 basePos, Vector2 baseSize);

	// 更新処理
	void Update(float deltaTime, DirectInput* input);

	// 描画処理
	void Draw();

	// パラメータのSetter
	void SetParam(std::variant<float, int> param) { param_ = param; }

	// アニメーション開始処理
	void StartAnimation() { animSprite_->StartAnimation(); }

private:
	// アニメーション用のスプライト
	std::unique_ptr<SpriteScaleWipeAnimator> animSprite_;

	// スコアテキスト
	std::unique_ptr<TinyEngine::Sprite> text_;

	Vector2 basePos_ = {300.0f, 400.0f};
	Vector2 baseSize_ = {400.0f, 100.0f};

	// 表示するパラメータ
	std::variant<float, int> param_;
};
