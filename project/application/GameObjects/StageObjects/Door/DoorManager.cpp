#include "DoorManager.h"
#include "JsonManager.h"

void DoorManager::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// JSON読み込み
	jsonPath_ = "Doors.json";
	LoadFromJson(jsonPath_);
}

void DoorManager::Update(float deltaTime) {
	for (auto& door : doors_) {
		door->Update(deltaTime);
	}
}

void DoorManager::Draw() {
	for (auto& door : doors_) {
		door->Draw();
	}
}

void DoorManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Door Editor");

	ImGui::Text("Door Count: %d", (int)doors_.size());

	// 壁追加
	if (ImGui::Button("Add Door")) {
		WallStatus s = {1.0f, 1.0f, 0.0f, 0.0f};
		auto door = std::make_unique<Door>();
		door->Initialize(ctx_, s);
		doors_.push_back(std::move(door));
	}

	// JSON保存ボタンに変更
	if (ImGui::Button("Save JSON")) {
		SaveToJson(jsonPath_);
	}

	ImGui::Separator();

	int index = 0;
	for (auto it = doors_.begin(); it != doors_.end();) {
		auto& door = *it;
		WallStatus& s = door->GetWallStatus();

		// ID 衝突防止
		ImGui::PushID(index);

		// 折りたたみヘッダー
		std::string header = "Door " + std::to_string(index);
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::DragFloat("Width", &s.width, 0.1f);
			ImGui::DragFloat("Depth", &s.depth, 0.1f);
			ImGui::DragFloat("CenterX", &s.centerX, 0.1f);
			ImGui::DragFloat("CenterZ", &s.centerZ, 0.1f);

			door->SetWallStatus(s);

			// 削除ボタン
			if (ImGui::Button("Delete")) {
				it = doors_.erase(it);
				ImGui::PopID();
				continue;
			}
		}

		ImGui::PopID();
		++it;
		++index;
	}

	ImGui::End();
#endif
}

void DoorManager::LoadFromJson(const std::string& filepath) {
	doors_.clear();

	// JSONからWallStatusの配列として読み込む
	std::vector<WallStatus> doorDataList;
	if (!JsonManager::Load(filepath, doorDataList)) {
		return;
	}

	// 読み込んだステータスからWallを生成
	for (const auto& s : doorDataList) {
		auto door = std::make_unique<Door>();
		door->Initialize(ctx_, s);
		doors_.push_back(std::move(door));
	}
}

void DoorManager::SaveToJson(const std::string& filepath) {
	// 現在ある壁のステータスリストを作成
	std::vector<WallStatus> doorDataList;
	for (auto& door : doors_) {
		doorDataList.push_back(door->GetWallStatus());
	}

	// JSONファイルへ保存
	JsonManager::Save(filepath, doorDataList);
}
