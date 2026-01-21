#include "WaveTimer.h"
#include <string>

void WaveTimer::Initialize(float maxTime, EngineContext* ctx) {
	for (int i=0;i<currentTimeSprites_.size();++i) {
		currentTimeSprites_[i] = std::make_unique<Sprite>();
		currentTimeSprites_[i]->Initialize(ctx, "resources/0.png");
		currentTimeSprites_[i]->SetSize({30.0f, 50.0f});
		currentTimeSprites_[i]->SetPosition({40.0f + i * 30.0f, 40.0f});
	}

	// 最大時間を設定
	currentTime_ = maxTime;

	// 現在の時間に応じてテクスチャを設定
	ChangeTexture();
}

void WaveTimer::Update(float deltaTime) {
	// 時間経過
	currentTime_ -= deltaTime;

	// 現在の時間に応じてテクスチャの変更
	ChangeTexture();

	// タイマーが0になったらフラグをたてる
	if (currentTime_ <= 0.0f) {
		isTimeUp_ = true;
	}

	/*for (auto& sprite : currentTimeSprites_) {
		sprite->Update();
	}*/
}

void WaveTimer::Draw() {
	/*for (auto& sprite : currentTimeSprites_) {
		sprite->Draw();
	}*/
}

void WaveTimer::ChangeTexture() {
	int tempTime = static_cast<int>(currentTime_);
	int tensPlace = tempTime / 10;
	int onesPlace = tempTime % 10;
	currentTimeSprites_[0]->SetTexture("resource/" + std::to_string(tensPlace) + ".png"); // 十の位
	currentTimeSprites_[1]->SetTexture("resource/" + std::to_string(onesPlace) + ".png"); // 一の位
}