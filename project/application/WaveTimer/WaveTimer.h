#pragma once
#include "Sprite.h"
#include "EngineContext.h"
#include <array>
#include <memory>

class WaveTimer {
public:
	void Initialize(float maxTime, EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

	// Getter
	bool GetIsTimeUp() const { return isTimeUp_; }

private:
	// 現在の時間に応じてテクスチャを変更
	void ChangeTexture();

private:
	// 経過時間
	float currentTime_;

	// 経過時間のスプライト
	std::array<std::unique_ptr<Sprite>, 2> currentTimeSprites_;

	// タイマーが0になったかどうか
	bool isTimeUp_ = false;
};
