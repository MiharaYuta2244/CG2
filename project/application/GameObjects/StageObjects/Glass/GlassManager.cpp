#include "JsonManager.h"
#include "GlassManager.h"

void GlassManager::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// JSON読み込み
	jsonPath_ = "Glasses.json";
	LoadFromJson(jsonPath_);
}

void GlassManager::Update() {
	for (auto& wall : glasses_) {
		wall->Update();
	}
}

void GlassManager::Draw() {
	for (auto& wall : glasses_) {
		wall->Draw();
	}
}

void GlassManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Glass Editor");

	ImGui::Text("Glass Count: %d", (int)glasses_.size());

	ImGui::Separator();

	int index = 0;
	for (auto it = glasses_.begin(); it != glasses_.end();) {
		auto& glass = *it;
		GlassStatus& s = glass->GetGlassStatus();

		s.width = glass->GetTransform().scale.x;
		s.depth = glass->GetTransform().scale.z;
		s.centerX = glass->GetTransform().translate.x;
		s.centerZ = glass->GetTransform().translate.z;

		// ID 衝突防止
		ImGui::PushID(index);

		// 折りたたみヘッダー
		std::string header = "Glass " + std::to_string(index);
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			bool isChanged = false;
			isChanged |= ImGui::DragFloat("Width", &s.width, 0.1f);
			isChanged |= ImGui::DragFloat("Depth", &s.depth, 0.1f);
			isChanged |= ImGui::DragFloat("CenterX", &s.centerX, 0.1f);
			isChanged |= ImGui::DragFloat("CenterZ", &s.centerZ, 0.1f);

			if (isChanged) {
				glass->SetGlassStatus(s);
			}

			// 削除ボタン
			if (ImGui::Button("Delete")) {
				it = glasses_.erase(it);
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
	if (ImGui::Button("Add Glass")) {
		GlassStatus s = {1.0f, 1.0f, 0.0f, 0.0f};
		auto wall = std::make_unique<Glass>();
		wall->Initialize(ctx_, s);
		glasses_.push_back(std::move(wall));
	}

	// JSON保存ボタンに変更
	if (ImGui::Button("Save JSON")) {
		SaveToJson(jsonPath_);
	}

	ImGui::End();
#endif
}

void GlassManager::LoadFromJson(const std::string& filepath) {
	glasses_.clear();

	// JSONからGlassStatusの配列として読み込む
	std::vector<GlassStatus> wallDataList;
	if (!JsonManager::Load(filepath, wallDataList)) {
		return;
	}

	// 読み込んだステータスからWallを生成
	for (const auto& s : wallDataList) {
		auto glass = std::make_unique<Glass>();
		glass->Initialize(ctx_, s);
		glasses_.push_back(std::move(glass));
	}
}

void GlassManager::SaveToJson(const std::string& filepath) {
	// 現在あるガラスのステータスリストを作成
	std::vector<GlassStatus> glassDataList;
	for (auto& wall : glasses_) {
		glassDataList.push_back(wall->GetGlassStatus());
	}

	// JSONファイルへ保存
	JsonManager::Save(filepath, glassDataList);
}

void GlassManager::RemoveGlass(Glass* glass) {
	for (auto it = glasses_.begin(); it != glasses_.end(); ++it) {
		if (it->get() == glass) {
			glasses_.erase(it);
			return;
		}
	}
}
