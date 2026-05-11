#include "ResultUIManager.h"

void ResultUIManager::Initialize(EngineContext* ctx) {
	// スコアテキスト生成&初期化
	scoreText_ = std::make_unique<ResultScoreText>();
	scoreText_->Initialize(ctx);

	// タイマーテキスト生成&初期化
	timerText_ = std::make_unique<ResultTimerText>();
	timerText_->Initialize(ctx);

	// リトライボタン生成&初期化
	retryButton_ = std::make_unique<RetryButton>();
	retryButton_->Initialize(ctx);

	// タイトルへ戻るボタン生成&初期化
	toTitleButton_ = std::make_unique<ToTitleButton>();
	toTitleButton_->Initialize(ctx);
}

void ResultUIManager::Update(float deltaTime, DirectInput* input) {
	// スコアテキスト更新
	scoreText_->Update(deltaTime, input);

	// タイマーテキスト更新
	timerText_->Update(deltaTime, input);

	// ボタンの選択
	int size = static_cast<int>(buttons_.size());
	if (input->KeyTriggered(DIK_D)) {
		selectedIndex_ = (selectedIndex_ + 1) % size;
	}
	if (input->KeyTriggered(DIK_A)) {
		selectedIndex_ = (selectedIndex_ - 1 + size) % size;
	}

	// 決定ボタンを押したら
	if (input->KeyTriggered(DIK_SPACE)) {
		OnDecide();
	}

	// ボタンの更新
	for (int i = 0; i < buttons_.size(); i++) {
		bool isSelected = (i == selectedIndex_);
		buttons_[i]->Update(isSelected);
	}
}

void ResultUIManager::Draw() {
	// スコアテキスト描画
	scoreText_->Draw();

	// タイマーテキスト描画
	timerText_->Draw();

	// ボタンの描画
	for (auto button : buttons_) {
		button->Draw();
	}
}

void ResultUIManager::RegisterButtons() {
	// すべてのボタンをリストに登録
	buttons_.push_back(retryButton_.get());
	buttons_.push_back(toTitleButton_.get());
}

void ResultUIManager::OnDecide() { buttons_[selectedIndex_]->Execute(); }
