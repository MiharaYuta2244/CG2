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

	// 座標のSetter
	void SetPos(Vector2 pos);

private:
	// スプライト
	std::unique_ptr<TinyEngine::Sprite> spriteFirst_;  // 1枚目
	std::unique_ptr<TinyEngine::Sprite> spriteSecond_; // 2枚目

	// 基準サイズ
	Vector2 startSize_ = {200, 50};
	Vector2 endSize_ = {1280, 0};
	Vector2 secondEndSize_ = {400, 150};

	// アニメーション用変数
	AnimationBundle<float> widthAnim_;  // 横幅
	AnimationBundle<float> heightAnim_; // 縦幅
	AnimationBundle<float> secondWidthAnim_;  // 横幅
	AnimationBundle<float> secondHeightAnim_; // 縦幅
	AnimationBundle<float> secondAlphaAnim_;  // 透明度

	// アニメーション開始フラグ
	bool isStartAnimation_ = false;

	Vector4 spriteFirstColor_ = {0.447f, 0.2156f, 1.0f, 1.0f};
	Vector4 spriteSecondColor_ = {0.447f, 0.2156f, 1.0f, 1.0f};
};
