#pragma once
#include "GameObjects/ResultUI/Animation/SpriteScaleWipeAnimator.h"
#include "Sprite.h"

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

	// クリアタイムのSetter
	void SetClearTime(float time) { clearTime_ = time; }

private:
	// アニメーション用のスプライト
	std::unique_ptr<SpriteScaleWipeAnimator> animSprite_;

	// スコアテキスト
	std::unique_ptr<TinyEngine::Sprite> text_;

	Vector2 basePos_ = {300.0f, 200.0f};
	Vector2 baseSize_ = {400.0f, 100.0f};

	// タイム
	float clearTime_ = 0.0f;
};
