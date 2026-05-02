#pragma once
#include "Sprite.h"

/// <summary>
/// フラッシュ演出
/// </summary>
class FlashEffect {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// フラッシュの開始処理
	void Trigger();

	// 演出が終了したかどうかのフラグ Getter
	bool Finish() const { return isFinished_; }

private:
	std::unique_ptr<TinyEngine::Sprite> sprite_;

	// 透明度
	float flashIntensity_ = 1.0f;

	// アクティブかどうか
	bool isActive_ = false;

	// 演出が終了下かどうか
	bool isFinished_ = false;
};
