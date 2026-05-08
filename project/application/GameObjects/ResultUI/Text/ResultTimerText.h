#pragma once
#include "Sprite.h"
#include "GameObjects/ResultUI/Animation/SpriteScaleWipeAnimator.h"

/// <summary>
/// リザルトシーンでクリアタイマーを表すテキスト
/// </summary>
class ResultTimerText {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, DirectInput* input);

	// 描画処理
	void Draw();

private:
	// アニメーション用のスプライト
	std::unique_ptr<SpriteScaleWipeAnimator> animSprite_;

	// スコアテキスト
	std::unique_ptr<TinyEngine::Sprite> text_;

	Vector2 basePos_ = {300.0f, 200.0f};
	Vector2 baseSize_ = {400.0f, 100.0f};
};
