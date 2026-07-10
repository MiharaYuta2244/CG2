#include "JsonManager.h"
#include "healAreaManager.h"

void HealAreaManager::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// JSON読み込み
	jsonPath_ = "HealAreas.json";
	LoadFromJson(jsonPath_);
}

void HealAreaManager::Update(float deltaTime) {
	for (auto& healArea : healAreas_) {
		healArea->Update(deltaTime);
	}
}

void HealAreaManager::Draw() {
	for (auto& healArea : healAreas_) {
		healArea->Draw();
	}
}

void HealAreaManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("HealArea Editor");

	ImGui::Text("HealArea Count: %d", (int)healAreas_.size());

	ImGui::Separator();

	int index = 0;
	for (auto it = healAreas_.begin(); it != healAreas_.end();) {
		auto& healArea = *it;
		HealAreaStatus& s = healArea->GetHealAreaStatus();

		s.width = healArea->GetTransform().scale.x;
		s.depth = healArea->GetTransform().scale.z;
		s.centerX = healArea->GetTransform().translate.x;
		s.centerZ = healArea->GetTransform().translate.z;

		// ID 衝突防止
		ImGui::PushID(index);

		// 折りたたみヘッダー
		std::string header = "HealArea " + std::to_string(index);
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			bool isChanged = false;
			isChanged |= ImGui::DragFloat("Width", &s.width, 0.1f);
			isChanged |= ImGui::DragFloat("Depth", &s.depth, 0.1f);
			isChanged |= ImGui::DragFloat("CenterX", &s.centerX, 0.1f);
			isChanged |= ImGui::DragFloat("CenterZ", &s.centerZ, 0.1f);

			if (isChanged) {
				healArea->SetHealAreaStatus(s);
			}

			// 削除ボタン
			if (ImGui::Button("Delete")) {
				it = healAreas_.erase(it);
				ImGui::PopID();
				continue;
			}
		}

		ImGui::PopID();
		++it;
		++index;
	}

	ImGui::Separator();

	// 回復エリア追加
	if (ImGui::Button("Add HealArea")) {
		HealAreaStatus s = {1.0f, 1.0f, 0.0f, 0.0f};
		auto healArea = std::make_unique<HealArea>();
		healArea->Initialize(ctx_, s);
		healAreas_.push_back(std::move(healArea));
	}

	// JSON保存ボタンに変更
	if (ImGui::Button("Save JSON")) {
		SaveToJson(jsonPath_);
	}

	ImGui::End();
#endif
}

void HealAreaManager::LoadFromJson(const std::string& filepath) {
	healAreas_.clear();

	// JSONからHealAreaStatusの配列として読み込む
	std::vector<HealAreaStatus> healAreaDataList;
	if (!JsonManager::Load(filepath, healAreaDataList)) {
		return;
	}

	// 読み込んだステータスからHealAreaを生成
	for (const auto& s : healAreaDataList) {
		auto healArea = std::make_unique<HealArea>();
		healArea->Initialize(ctx_, s);
		healAreas_.push_back(std::move(healArea));
	}
}

void HealAreaManager::SaveToJson(const std::string& filepath) {
	// 現在ある回復エリアのステータスリストを作成
	std::vector<HealAreaStatus> healAreaDataList;
	for (auto& healArea : healAreas_) {
		healAreaDataList.push_back(healArea->GetHealAreaStatus());
	}

	// JSONファイルへ保存
	JsonManager::Save(filepath, healAreaDataList);
}
