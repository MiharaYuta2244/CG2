#pragma once
#include "AnimationBundle.h"
#include "Sprite.h"

/// <summary>
/// タイトル、リザルトで決定ボタンを押したときに出るエフェクト
/// </summary>
class DecideEffect {
public:
	void Initialize(EngineContext* ctx, Vector2 pos);

	void Update(float deltaTime);

	void Draw();

	// アニメーション開始処理
	void StartAnimation();

private:
	// スプライト
	std::unique_ptr<TinyEngine::Sprite> spriteFirst_;  // 1枚目
	std::unique_ptr<TinyEngine::Sprite> spriteSecond_; // 2枚目

	// 基準サイズ
	Vector2 startSize_ = {200, 50};
	Vector2 endSize_ = {1280, 0};

	// アニメーション用変数
	AnimationBundle<float> widthAnim_;  // 横幅
	AnimationBundle<float> heightAnim_; // 縦幅

	// アニメーション開始フラグ
	bool isStartAnimation_ = false;

	Vector4 spriteFirstColor_ = {0.447f, 0.2156f, 1, 1};
};
