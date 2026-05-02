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

void MenuList::Update(DirectInput* input, float deltaTime) {
	int index = static_cast<int>(items_.size());

	if (input->KeyTriggered(DIK_W) || input->KeyTriggered(DIK_UP)) {
		currentIndex_ = (currentIndex_ - 1 + index) % index;
	}
	if (input->KeyTriggered(DIK_S) || input->KeyTriggered(DIK_DOWN)) {
		currentIndex_ = (currentIndex_ + 1) % index;
	}
	if (input->KeyTriggered(DIK_SPACE) || input->KeyTriggered(DIK_Z)) {
		items_[currentIndex_].onSelect();
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
}

void MenuList::Draw() {
	background_->Draw();
	titleLogo_->Draw();

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
