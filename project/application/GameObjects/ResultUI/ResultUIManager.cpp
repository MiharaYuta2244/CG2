#include "ResultUIManager.h"

void ResultUIManager::Initialize(EngineContext* ctx, CommonData* commonData) {
	// スコアテキスト生成&初期化
	scoreText_ = std::make_unique<ResultParamText>();
	scoreText_->Initialize(ctx, {300, 400}, {400, 100});

	// タイマーテキスト生成&初期化
	timerText_ = std::make_unique<ResultParamText>();
	timerText_->Initialize(ctx, {300, 200}, {400, 100});

	// リトライボタン生成&初期化
	retryButton_ = std::make_unique<RetryButton>();
	retryButton_->Initialize(ctx);

	// タイトルへ戻るボタン生成&初期化
	toTitleButton_ = std::make_unique<ToTitleButton>();
	toTitleButton_->Initialize(ctx);

	// キル数の受け渡し
	scoreText_->SetParam(commonData->killCount);

	// クリア時間の受け渡し
	timerText_->SetParam(commonData->clearTime);
}

void ResultUIManager::Update(float deltaTime, DirectInput* input, GamePad* gamePad) {
	// スコアテキスト更新
	scoreText_->Update(deltaTime, input);

	// タイマーテキスト更新
	timerText_->Update(deltaTime, input);

	// スティック制御用クールダウン
	stickCooldown_ -= deltaTime;

	// ボタンの選択
	bool right = input->KeyTriggered(DIK_D);
	bool left = input->KeyTriggered(DIK_A);
	bool decide = input->KeyTriggered(DIK_SPACE);

	// パッド入力
	if (gamePad && gamePad->GetState().connected) {
		const auto& pad = gamePad->GetState();

		float lx = pad.axes.lx;

		// スティックが一定以上倒されたら
		if (!stickInUse_) {
			if (lx > 0.9f) {
				right = true;
				stickInUse_ = true;
			} else if (lx < -0.9f) {
				left = true;
				stickInUse_ = true;
			}
		}

		// スティックがニュートラルに戻ったら再入力可能に
		if (std::abs(lx) < 0.3f) {
			stickInUse_ = false;
		}

		if (pad.buttonsPressed.a) {
			decide = true;
		}

		if (pad.buttonsPressed.dpadLeft) {
			left = true;
		}

		if (pad.buttonsPressed.dpadRight) {
			right = true;
		}
	}

	int size = static_cast<int>(buttons_.size());
	if (right) {
		selectedIndex_ = (selectedIndex_ + 1) % size;
	}
	if (left) {
		selectedIndex_ = (selectedIndex_ - 1 + size) % size;
	}

	// 決定ボタンを押したら
	if (decide && !isDecided_) {
		OnDecide();
		isDecided_ = true;
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
