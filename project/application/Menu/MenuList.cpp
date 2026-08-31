#include "MenuList.h"
#include "MathOperator.h"

using namespace TinyEngine;

void MenuList::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// 決定ボタン入力時のエフェクト
	decideEffect_ = std::make_unique<DecideEffect>();
	decideEffect_->Initialize(ctx, startPos_);

	// オーディオマネージャー生成&初期化
	audioManager_ = std::make_unique<AudioManager>();
	audioManager_->Initialize();
	audioManager_->LoadWave("Select", "resources/sounds/se/Select.mp3");
	audioManager_->LoadWave("Decide", "resources/sounds/se/Decide.mp3");

	// 選択中背景スプライトの生成&初期化
	selectorBg_ = std::make_unique<Sprite>();
	selectorBg_->Initialize(ctx_, "white.png");
	selectorBg_->SetColor({1.0f, 1.0f, 1.0f, 0.2f});
	selectorBg_->SetSize(selectorBgSize_);
	selectorBg_->SetAnchorPoint({0.5f, 0.0f});
}

void MenuList::AddItem(const std::string& label, const std::string& texturePath, std::function<void()> onSelect) {
	MenuItem item;
	item.label = label;
	item.onSelect = onSelect;
	item.sprite = std::make_unique<Sprite>();
	item.sprite->Initialize(ctx_, texturePath);
	item.originalSize = item.sprite->GetSize();
	items_.push_back(std::move(item));
}

void MenuList::AddToggleItem(const std::string& label, const std::string& texturePath, bool* target) {
	MenuItem item;
	item.label = label;
	item.toggleValue = target;
	item.onSelect = [target]() { *target = !(*target); };

	item.sprite = std::make_unique<Sprite>();
	item.sprite->Initialize(ctx_, texturePath);
	item.originalSize = item.sprite->GetSize();

	item.checkIcon = std::make_unique<Sprite>();
	item.checkIcon->Initialize(ctx_, "Check.png");

	items_.push_back(std::move(item));
}

void MenuList::Update(DirectInput* input, GamePad* gamePad, float deltaTime) {
	int index = static_cast<int>(items_.size());

	// スティック制御用クールダウン
	stickCooldown_ -= deltaTime;

	// ボタンの選択
	bool up = input->KeyTriggered(DIK_W);
	bool down = input->KeyTriggered(DIK_S);
	bool decide = input->KeyTriggered(DIK_SPACE);

	// パッド入力
	if (gamePad && gamePad->GetState().connected) {
		const auto& pad = gamePad->GetState();

		float ly = pad.axes.ly;

		// スティックが一定以上倒されたら
		if (!stickInUse_) {
			if (ly < -0.9f) {
				up = true;
				stickInUse_ = true;
			} else if (ly > 0.9f) {
				down = true;
				stickInUse_ = true;
			}
		}

		// スティックがニュートラルに戻ったら再入力可能に
		if (std::abs(ly) < 0.3f) {
			stickInUse_ = false;
		}

		if (pad.buttonsPressed.a) {
			decide = true;
		}

		if (pad.buttonsPressed.dpadUp) {
			up = true;
		}

		if (pad.buttonsPressed.dpadDown) {
			down = true;
		}
	}

	// 決定アニメーション再生中は他の項目へ移動させない
	if (up && !isDecided_) {
		currentIndex_ = (currentIndex_ - 1 + index) % index;
		audioManager_->PlaySE("Select", 0.5f);
	}
	if (down && !isDecided_) {
		currentIndex_ = (currentIndex_ + 1) % index;
		audioManager_->PlaySE("Select", 0.5f);
	}

	// 決定処理
	if (decide && !isDecided_) {
		items_[currentIndex_].onSelect();
		audioManager_->PlaySE("Decide", 0.5f);

		if (items_[currentIndex_].toggleValue) {
			// トグル項目はロックせず即座に再入力可能にする
		} else {
			decideEffect_->StartAnimation();
			isDecided_ = true;
		}
	}

	// 背景スプライトの座標を選択中アイテムに合わせる
	selectorBg_->SetPosition({startPos_.x, startPos_.y + offsetY_ * currentIndex_});
	selectorBg_->Update();

	// 決定ボタン入力時エフェクト更新
	decideEffect_->Update(deltaTime);

	// 選択中メニューの座標をエフェクトに適用
	Vector2 effectPos = {startPos_.x, startPos_.y + offsetY_ * currentIndex_};
	decideEffect_->SetPos(effectPos);

	// 音声更新
	audioManager_->Update();
}

void MenuList::Draw() {
	// 背景
	selectorBg_->Draw();

	// 決定時エフェクト
	decideEffect_->Draw();

	for (int i = 0; i < items_.size(); i++) {
		auto& item = items_[i];
		auto& sprite = item.sprite;

		// 選択中は色を変える
		if (i == currentIndex_) {
			sprite->SetColor(selectColor_);
		} else {
			sprite->SetColor(normalColor_);
		}

		sprite->SetPosition({startPos_.x, startPos_.y + offsetY_ * i});
		sprite->Update();
		sprite->Draw();

		// トグル項目ならON/OFF状態のアイコンも描画
		if (item.toggleValue && item.checkIcon) {
			bool isOn = *item.toggleValue;
			item.checkIcon->SetColor(isOn ? Vector4{0.3f, 1.0f, 0.3f, 1.0f} : Vector4{0.4f, 0.4f, 0.4f, 0.5f});
			item.checkIcon->SetPosition({startPos_.x + checkIconOffsetX_, startPos_.y + offsetY_ * i});
			item.checkIcon->Update();
			item.checkIcon->Draw();
		}
	}
}
