#include "DoorManager.h"
#include "JsonManager.h"

void DoorManager::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// JSON読み込み
	jsonPath_ = "Doors.json";
	LoadFromJson(jsonPath_);
}

void DoorManager::Update(float deltaTime, Vector3 playerPos) {
	for (auto& door : doors_) {
		door->Update(deltaTime, playerPos);
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
		DoorStatus s = {1.0f, 1.0f, 0.0f, 0.0f};
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
		DoorStatus& s = door->GetDoorStatus();

		s.width = door->GetTransform().scale.x;
		s.depth = door->GetTransform().scale.z;
		s.centerX = door->GetTransform().translate.x;
		s.centerZ = door->GetTransform().translate.z;
		s.rotateY = door->GetTransform().rotate.y;

		// ID 衝突防止
		ImGui::PushID(index);

		// 折りたたみヘッダー
		std::string header = "Door " + std::to_string(index);
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			bool isChanged = false;
			isChanged |= ImGui::DragFloat("Width", &s.width, 0.1f);
			isChanged |= ImGui::DragFloat("Depth", &s.depth, 0.1f);
			isChanged |= ImGui::DragFloat("CenterX", &s.centerX, 0.1f);
			isChanged |= ImGui::DragFloat("CenterZ", &s.centerZ, 0.1f);
			isChanged |= ImGui::DragFloat("RotateY", &s.rotateY, 0.1f);

			if (isChanged) {
				door->SetDoorStatus(s);
			}

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

	// JSONからDoorStatusの配列として読み込む
	std::vector<DoorStatus> doorDataList;
	if (!JsonManager::Load(filepath, doorDataList)) {
		return;
	}

	// 読み込んだステータスからDoorを生成
	for (const auto& s : doorDataList) {
		auto door = std::make_unique<Door>();
		door->Initialize(ctx_, s);
		doors_.push_back(std::move(door));
	}
}

void DoorManager::SaveToJson(const std::string& filepath) {
	// 現在あるドアのステータスリストを作成
	std::vector<DoorStatus> doorDataList;
	for (auto& door : doors_) {
		doorDataList.push_back(door->GetDoorStatus());
	}

	// JSONファイルへ保存
	JsonManager::Save(filepath, doorDataList);
}
