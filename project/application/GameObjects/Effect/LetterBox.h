#pragma once
#include "EasingAnimation.h"
#include "Sprite.h"

/// <summary>
/// レターボックス演出
/// </summary>
class LetterBox {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// 演出の開始処理
	void Trigger();

	// 開始トリガー Getter
	bool GetIsTriggered() const { return isTriggered_; }

	// イージングアクティブ状態 Getter
	bool GetIsActive() const { return easing_.GetIsActive(); }

private:
	std::array<std::unique_ptr<TinyEngine::Sprite>, 2> sprites_;
	Vector2 positionTop_;
	Vector2 positionBottom_;
	EasingAnimation<float> easing_;

	float progress_ = 0.0f;
	bool isTriggered_ = false;

	float screenWidth_ = 1280.0f;
	float screenHeight_ = 720.0f;
	float boxHeight_ = 100.0f;
};
