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

	if (up) {
		currentIndex_ = (currentIndex_ - 1 + index) % index;

		// SE再生
		audioManager_->PlaySE("Select", 0.5f);
	}
	if (down) {
		currentIndex_ = (currentIndex_ + 1) % index;

		// SE再生
		audioManager_->PlaySE("Select", 0.5f);
	}
	if (decide) {
		items_[currentIndex_].onSelect();
		decideEffect_->StartAnimation();

		// SE再生
		audioManager_->PlaySE("Decide", 0.5f);
	}

	// 決定ボタン入力時エフェクト更新
	decideEffect_->Update(deltaTime);

	// 選択中メニューの座標をエフェクトに適用
	Vector2 effectPos = {startPos_.x, startPos_.y + offsetY_ * currentIndex_};
	decideEffect_->SetPos(effectPos);

	// 音声更新
	audioManager_->Update();
}

void MenuList::Draw() {
	decideEffect_->Draw();

	for (int i = 0; i < items_.size(); i++) {
		auto& sprite = items_[i].sprite;

		// 選択中は色を変える
		if (i == currentIndex_) {
			sprite->SetColor(selectColor_);
			sprite->SetSize(items_[i].originalSize * 1.1f);
		} else {
			sprite->SetColor(normalColor_);
			sprite->SetSize(items_[i].originalSize);
		}

		sprite->SetPosition({startPos_.x, startPos_.y + offsetY_ * i});
		sprite->Update();
		sprite->Draw();
	}
}
