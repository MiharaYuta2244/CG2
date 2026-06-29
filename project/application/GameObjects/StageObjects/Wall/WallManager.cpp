#include "WallManager.h"
#include "JsonManager.h"

void WallManager::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// JSON読み込み
	jsonPath_ = "Walls.json";
	LoadFromJson(jsonPath_);
}

void WallManager::Update(float deltaTime) {
	for (auto& wall : walls_) {
		wall->Update(deltaTime);
	}
}

void WallManager::Draw() {
	for (auto& wall : walls_) {
		wall->Draw();
	}
}

void WallManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Wall Editor");

	ImGui::Text("Wall Count: %d", (int)walls_.size());

	ImGui::Separator();

	int index = 0;
	for (auto it = walls_.begin(); it != walls_.end();) {
		auto& wall = *it;
		WallStatus& s = wall->GetWallStatus();

		s.width = wall->GetTransform().scale.x;
		s.depth = wall->GetTransform().scale.z;
		s.centerX = wall->GetTransform().translate.x;
		s.centerZ = wall->GetTransform().translate.z;

		// ID 衝突防止
		ImGui::PushID(index);

		// 折りたたみヘッダー
		std::string header = "Wall " + std::to_string(index);
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			bool isChanged = false;
			isChanged |= ImGui::DragFloat("Width", &s.width, 0.1f);
			isChanged |= ImGui::DragFloat("Depth", &s.depth, 0.1f);
			isChanged |= ImGui::DragFloat("CenterX", &s.centerX, 0.1f);
			isChanged |= ImGui::DragFloat("CenterZ", &s.centerZ, 0.1f);

			if (isChanged) {
				wall->SetWallStatus(s);
			}

			// 削除ボタン
			if (ImGui::Button("Delete")) {
				it = walls_.erase(it);
				ImGui::PopID();
				continue;
			}
		}

		ImGui::PopID();
		++it;
		++index;
	}

	ImGui::Separator();

	// 壁追加
	if (ImGui::Button("Add Wall")) {
		WallStatus s = {1.0f, 1.0f, 0.0f, 0.0f};
		auto wall = std::make_unique<Wall>();
		wall->Initialize(ctx_, s);
		walls_.push_back(std::move(wall));
	}

	// JSON保存ボタンに変更
	if (ImGui::Button("Save JSON")) {
		SaveToJson(jsonPath_);
	}

	ImGui::End();
#endif
}

void WallManager::LoadFromJson(const std::string& filepath) {
	walls_.clear();

	// JSONからWallStatusの配列として読み込む
	std::vector<WallStatus> wallDataList;
	if (!JsonManager::Load(filepath, wallDataList)) {
		return;
	}

	// 読み込んだステータスからWallを生成
	for (const auto& s : wallDataList) {
		auto wall = std::make_unique<Wall>();
		wall->Initialize(ctx_, s);
		walls_.push_back(std::move(wall));
	}
}

void WallManager::SaveToJson(const std::string& filepath) {
	// 現在ある壁のステータスリストを作成
	std::vector<WallStatus> wallDataList;
	for (auto& wall : walls_) {
		wallDataList.push_back(wall->GetWallStatus());
	}

	// JSONファイルへ保存
	JsonManager::Save(filepath, wallDataList);
}
