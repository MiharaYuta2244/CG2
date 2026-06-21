#include "MenuList.h"
#include "MathOperator.h"

using namespace TinyEngine;

void MenuList::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// タイトルロゴの生成&初期化
	titleLogo_ = std::make_unique<Sprite>();
	titleLogo_->Initialize(ctx, "Title_Logo.png");
	titleLogo_->SetPosition(titleLogoPos_);

	// 背景の生成&初期化
	background_ = std::make_unique<Sprite>();
	background_->Initialize(ctx, "white.png");
	background_->SetSize({1280.0f, 720.0f});
	background_->SetColor(backgroundColor_);
	background_->SetEnableVoronoi(true);
	background_->SetVoronoiColor(voronoiColor_);
	background_->SetZDepth(100.0f);

	// 決定ボタン入力時のエフェクト
	decideEffect_ = std::make_unique<DecideEffect>();
	decideEffect_->Initialize(ctx, startPos_);
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
	}
	if (down) {
		currentIndex_ = (currentIndex_ + 1) % index;
	}
	if (decide) {
		items_[currentIndex_].onSelect();
		decideEffect_->StartAnimation();
	}

#ifdef USE_IMGUI
	ImGui::Begin("TitleLayout");
	ImGui::DragFloat2("Pos", &titleLogoPos_.x, 1.0f);
	ImGui::DragFloat2("StartPos", &startPos_.x, 1.0f);
	ImGui::DragFloat("OffsetY", &offsetY_, 1.0f);
	ImGui::End();

	ImGui::Begin("Voronoi");
	ImGui::DragFloat3("params", &voronoiParams_.x, 0.01f);
	ImGui::End();

	ImGui::Begin("Color");
	ImGui::ColorEdit4("Logo", &logoColor_.x);
	ImGui::ColorEdit4("Normal", &normalColor_.x);
	ImGui::ColorEdit4("Select", &selectColor_.x);
	ImGui::ColorEdit4("voronoi", &voronoiColor_.x);
	ImGui::ColorEdit4("background", &backgroundColor_.x);
	ImGui::End();
#endif

	// 背景更新
	voronoiTimer_ += deltaTime;
	background_->SetVoronoiParams(voronoiParams_.x, voronoiParams_.y, voronoiParams_.z, voronoiTimer_);
	background_->SetColor(backgroundColor_);
	background_->SetVoronoiColor(voronoiColor_);
	background_->Update();

	// タイトルロゴ更新
	titleLogo_->SetPosition(titleLogoPos_);
	titleLogo_->SetColor(logoColor_);
	titleLogo_->Update();

	// 決定ボタン入力時エフェクト更新
	decideEffect_->Update(deltaTime);
}

void MenuList::Draw() {
	background_->Draw();
	titleLogo_->Draw();
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
