#include "CageManager.h"
#include "JsonManager.h"

void CageManager::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// JSON読み込み
	jsonPath_ = "Cages.json";
	LoadFromJson(jsonPath_);
}

void CageManager::Update(float deltaTime) {
	for (auto& cage : cages_) {
		cage->Update(deltaTime);
	}
}

void CageManager::Draw() {
	for (auto& cage : cages_) {
		cage->Draw();
	}
}

void CageManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Cage Editor");

	ImGui::Text("Cage Count: %d", (int)cages_.size());
	ImGui::Separator();

	int index = 0;
	for (auto it = cages_.begin(); it != cages_.end();) {
		auto& cage = *it;
		CageStatus& s = cage->GetCageStatus();

		s.width = cage->GetTransform().scale.x;
		s.depth = cage->GetTransform().scale.z;
		s.centerX = cage->GetTransform().translate.x;
		s.centerZ = cage->GetTransform().translate.z;
		s.rotateY = cage->GetTransform().rotate.y;

		ImGui::PushID(index);

		std::string header = "Cage " + std::to_string(index);
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			bool isChanged = false;
			isChanged |= ImGui::DragFloat("Width", &s.width, 0.1f);
			isChanged |= ImGui::DragFloat("Depth", &s.depth, 0.1f);
			isChanged |= ImGui::DragFloat("CenterX", &s.centerX, 0.1f);
			isChanged |= ImGui::DragFloat("CenterZ", &s.centerZ, 0.1f);
			isChanged |= ImGui::DragFloat("RotateY", &s.rotateY, 0.1f);

			if (isChanged) {
				cage->SetCageStatus(s);
			}

			if (ImGui::Button("Delete")) {
				it = cages_.erase(it);
				ImGui::PopID();
				continue;
			}
		}

		ImGui::PopID();
		++it;
		++index;
	}

	ImGui::Separator();

	// Cage追加
	if (ImGui::Button("Add Cage")) {
		CageStatus s = {1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
		auto cage = std::make_unique<Cage>();
		cage->Initialize(ctx_, s);
		cages_.push_back(std::move(cage));
	}

	// JSON保存
	if (ImGui::Button("Save JSON")) {
		SaveToJson(jsonPath_);
	}

	ImGui::End();
#endif
}

void CageManager::LoadFromJson(const std::string& filepath) {
	cages_.clear();

	std::vector<CageStatus> cageDataList;
	if (!JsonManager::Load(filepath, cageDataList)) {
		return;
	}

	for (const auto& s : cageDataList) {
		auto cage = std::make_unique<Cage>();
		cage->Initialize(ctx_, s);
		cages_.push_back(std::move(cage));
	}
}

void CageManager::SaveToJson(const std::string& filepath) {
	std::vector<CageStatus> cageDataList;
	for (auto& cage : cages_) {
		cageDataList.push_back(cage->GetCageStatus());
	}

	JsonManager::Save(filepath, cageDataList);
}
