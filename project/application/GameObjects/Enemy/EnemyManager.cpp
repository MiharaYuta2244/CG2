#include "EnemyManager.h"
#include "JsonManager.h"

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

using namespace TinyEngine;

void EnemyManager::Initialize(EngineContext* ctx, BloodDecalManager* bloodDecalManager) {
	ctx_ = ctx;
	bloodDecalManager_ = bloodDecalManager;

	// JSON読み込み
	jsonPath_ = "Enemies.json";
	LoadFromJson(jsonPath_);
}

void EnemyManager::Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager, DoorManager* doorManager, GlassManager* glassManager) {
	// 死亡した敵をリストから削除
	enemies_.remove_if([](const std::unique_ptr<Enemy>& enemy) { return enemy->IsDead(); });

	// 生きている敵をすべて更新
	for (auto& enemy : enemies_) {
		if (enemy->GetIsMove()) {
			enemy->Update(deltaTime, player, enemyBulletManager, wallManager, doorManager, glassManager);
		}
	}
}

void EnemyManager::PostUpdate() {
	for (auto& enemy : enemies_) {
		enemy->PostUpdate();
	}
}

void EnemyManager::Draw() {
	for (auto& enemy : enemies_) {
		enemy->Draw();
	}
}

void EnemyManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Enemy Manager");

	ImGui::Text("Enemy Count: %d", (int)enemies_.size());

	ImGui::Separator();

	int index = 0;
	for (auto it = enemies_.begin(); it != enemies_.end();) {
		auto& enemy = *it;

		ImGui::PushID(index);
		std::string header = "Enemy " + std::to_string(index);

		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			// --- Position ---
			Vector3& pos = enemy->GetPos();
			ImGui::DragFloat3("Position", &pos.x, 0.1f);

			// --- Rotation ---
			Vector3 rot = enemy->GetRotate();
			if (ImGui::DragFloat3("Rotation", &rot.x, 0.5f)) {
				enemy->SetRotate(rot);
			}

			// 敵の削除ボタン
			if (ImGui::Button("Delete")) {
				it = enemies_.erase(it);
				ImGui::PopID();
				continue;
			}
		}

		++it;
		++index;
		ImGui::PopID();
	}

	ImGui::Separator();

	// 敵の追加ボタン
	if (ImGui::Button("Add Enemy")) {
		auto newEnemy = std::make_unique<Enemy>();
		newEnemy->Initialize(ctx_, {0.0f, 0.0f, 0.0f}, static_cast<EnemyType>(RandomUtils::RangeInt(0, 1)) ,bloodDecalManager_);
		newEnemy->SetPos({0.0f, 0.0f, 0.0f});
		newEnemy->SetRotate({0.0f, 0.0f, 0.0f});
		enemies_.push_back(std::move(newEnemy));
	}

	// JSON保存ボタン
	if (ImGui::Button("Save JSON")) {
		SaveToJson(jsonPath_);
	}

	ImGui::End();
#endif
}

void EnemyManager::SetMove() {
	for (auto& enemy : enemies_) {
		enemy->SetIsMove(true);
	}
}

void EnemyManager::SetStop() {
	for (auto& enemy : enemies_) {
		enemy->SetIsMove(false);
	}
}

void EnemyManager::LoadFromJson(const std::string& filepath) {
	enemies_.clear();

	std::vector<EnemyData> enemyDatas;
	if (!JsonManager::Load(filepath, enemyDatas)) {
		return;
	}

	for (const auto& data : enemyDatas) {
		auto enemy = std::make_unique<Enemy>();
		enemy->Initialize(ctx_, data.pos, static_cast<EnemyType>(RandomUtils::RangeInt(0, 1)), bloodDecalManager_);
		enemy->SetPos(data.pos);
		enemy->SetRotate(data.rot);
		enemies_.push_back(std::move(enemy));
	}
}

void EnemyManager::SaveToJson(const std::string& filepath) {
	std::vector<EnemyData> enemyDatas;

	for (auto& enemy : enemies_) {
		EnemyData data;
		data.pos = enemy->GetPos();
		data.rot = enemy->GetRotate();
		enemyDatas.push_back(data);
	}

	JsonManager::Save(filepath, enemyDatas);
}