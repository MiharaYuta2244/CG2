#include "ResultUIManager.h"

void ResultUIManager::Initialize(EngineContext* ctx) {
	// スコアテキスト生成&初期化
	scoreText_=std::make_unique<ResultScoreText>();
	scoreText_->Initialize(ctx);

	// タイマーテキスト生成&初期化
	timerText_=std::make_unique<ResultTimerText>();
	timerText_->Initialize(ctx);
}

void ResultUIManager::Update(float deltaTime, DirectInput* input) {
	// スコアテキスト更新
	scoreText_->Update(deltaTime, input);

	// タイマーテキスト更新
	timerText_->Update(deltaTime, input);
}

void ResultUIManager::Draw() {
	// スコアテキスト描画
	scoreText_->Draw();

	// タイマーテキスト描画
	timerText_->Draw();
}
