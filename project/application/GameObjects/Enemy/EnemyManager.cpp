#include "EnemyManager.h"
#include "JsonManager.h"

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

using namespace TinyEngine;

void EnemyManager::Initialize(EngineContext* ctx, DecalManager* bloodDecalManager) {
	ctx_ = ctx;
	bloodDecalManager_ = bloodDecalManager;

	// オーディオマネージャー生成&初期化
	audioManager_ = std::make_unique<AudioManager>();
	audioManager_->Initialize();
	audioManager_->LoadWave("Cymbal", "resources/sounds/se/Cymbal.mp3");
	audioManager_->LoadWave("Snare", "resources/sounds/se/Snare.mp3");
	audioManager_->LoadWave("Shot", "resources/sounds/se/Shot.mp3");

	// JSON読み込み
	jsonPath_ = "Enemies.json";
	LoadFromJson(jsonPath_);
}

void EnemyManager::Update(
    float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager, DoorManager* doorManager, GlassManager* glassManager, EnemyBombManager* enemyBombManager) {

	// 削除前の敵の数を取得
	size_t beforeCount = enemies_.size();

	// 死亡した敵をリストから削除
	enemies_.remove_if([](const std::unique_ptr<Enemy>& enemy) { return enemy->IsDead(); });

	// 削除後の敵の数を取得
	size_t afterCount = enemies_.size();

	// 数が減ったらSEを再生
	if (beforeCount > afterCount) {
		audioManager_->PlaySE("Cymbal", 0.2f);
		audioManager_->PlaySE("Snare", 0.2f);
	}

	// 生きている敵をすべて更新
	for (auto& enemy : enemies_) {
		enemy->Update(deltaTime, player, enemyBulletManager, wallManager, doorManager, glassManager, enemyBombManager);
	}

	// 音声更新
	audioManager_->Update();
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

			// --- isMove ---
			bool isMove = enemy->GetIsMove();
			if (ImGui::Checkbox("Is Move", &isMove)) {
				enemy->SetIsMove(isMove);
			}

			// --- Type ---
			int currentType = static_cast<int>(enemy->GetEnemyType());
			const char* typeNames[] = {"Normal", "Shotgun", "Bomber"};
			if (ImGui::Combo("Type", &currentType, typeNames, IM_ARRAYSIZE(typeNames))) {
				enemy->SetEnemyType(static_cast<EnemyType>(currentType));
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
		newEnemy->Initialize(ctx_, {0.0f, 0.0f, 0.0f}, EnemyType::Normal, bloodDecalManager_, audioManager_.get());
		newEnemy->SetPos({0.0f, 0.0f, 0.0f});
		newEnemy->SetRotate({0.0f, 0.0f, 0.0f});
		newEnemy->SetIsMove(false);
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
		enemy->Initialize(ctx_, data.pos, static_cast<EnemyType>(data.type), bloodDecalManager_, audioManager_.get());
		enemy->SetPos(data.pos);
		enemy->SetRotate(data.rot);
		enemy->SetIsMove(data.isMove);
		enemies_.push_back(std::move(enemy));
	}
}

void EnemyManager::SaveToJson(const std::string& filepath) {
	std::vector<EnemyData> enemyDatas;

	for (auto& enemy : enemies_) {
		EnemyData data;
		data.pos = enemy->GetPos();
		data.rot = enemy->GetRotate();
		data.isMove = enemy->GetIsMove();
		data.type = static_cast<int>(enemy->GetEnemyType());
		enemyDatas.push_back(data);
	}

	JsonManager::Save(filepath, enemyDatas);
}

void EnemyManager::AllDead() {
	for (auto& enemy : enemies_) {
		enemy->Dead();
	}
}