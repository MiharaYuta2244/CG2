#include "PlayerHealth.h"
#include "Logger.h"
#include "imGuiManager/ImGuiManager.h"

void PlayerHealth::Initialize(float maxHP) {
	// HPの設定
	maxHP_ = maxHP;
	hp_ = maxHP;
}

void PlayerHealth::Update(float deltaTime) {
	// 無敵タイマー更新
	if (isInvincible_) {
		invincibleTimer_.Update(deltaTime);
	}

	// 無敵時間が終了したらフラグを下す
	if (invincibleTimer_.IsEnd()) {
		isInvincible_ = false;
	}

	// プレイヤーのHPが0なら一度だけログ出力
	if (preHP_ <= 0.0f && hp_ <= 0.0f) {
		Logger::Log("Player is Dead", LogLevel::Info);
	}

	// 前フレームHPに記録
	preHP_ = hp_;
}

void PlayerHealth::Damage(float value) {
	if (isInvincible_) {
		return;
	}

	// ダメージ処理
	hp_ -= value;

	// 無敵状態へ
	isInvincible_ = true;

	// タイマー設定
	invincibleTimer_.Initialize(invincibleDuration_);
}

void PlayerHealth::Heal(float value) {
	hp_ += value;
	hp_ = std::min(hp_, maxHP_);
}

void PlayerHealth::AllHeal() {
	// HP全回復
	hp_ = maxHP_;
}

void PlayerHealth::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("PlayerHP");
	ImGui::DragFloat("currentHP", &hp_, 1.0f);
	ImGui::DragFloat("maxHP", &maxHP_, 1.0f);
	ImGui::End();
#endif // USE_IMGUI
}

bool PlayerHealth::IsDead() const {
	if (hp_ <= 0.0f) {
		return true;
	}

	return false;
}
