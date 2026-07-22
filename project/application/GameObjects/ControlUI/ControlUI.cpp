#include "ControlUI.h"
#include "ColorPalette.h"

using namespace TinyEngine;

void ControlUI::Initialize(EngineContext* ctx, DecalManager* uiDecalManager) {
	uiDecalManager_ = uiDecalManager;
	attackUIPos_ = {0.0f, 0.1f, 3.0f};
	holdUIPos_ = {261.0f, 0.1f, 4.0f};
}

void ControlUI::Update() {
	// ImGui等で変更されたUI位置をデカールのTransformへ反映
	if (attackDecal_) {
		attackDecal_->transform.translate = {attackUIPos_.x, 0.1f, attackUIPos_.z};
	}
	if (holdDecal_) {
		holdDecal_->transform.translate = {holdUIPos_.x, 0.1f, holdUIPos_.z};
	}
}

void ControlUI::AddAttackUIDecal(Vector3 scale) { attackDecal_ = uiDecalManager_->AddDecal("white.png", attackUIPos_, {std::numbers::pi_v<float> / 2.0f, 0, 0}, scale, ColorPalette::White()); }

void ControlUI::AddHoldUIDecal(Vector3 scale) { holdDecal_ = uiDecalManager_->AddDecal("white.png", holdUIPos_, {std::numbers::pi_v<float> / 2.0f, 0, 0}, scale, ColorPalette::White()); }

void ControlUI::DrawImGui() {
	if (ImGui::Begin("Control UI")) {

		ImGui::Text("Attack UI Position");
		ImGui::DragFloat3("Attack Pos", &attackUIPos_.x, 0.01f);

		ImGui::Separator();

		ImGui::Text("Hold UI Position");
		ImGui::DragFloat3("Hold Pos", &holdUIPos_.x, 0.01f);
	}
	ImGui::End();
}