#include "TestFunction.h"

TestFunction* TestFunction::GetInstance() {
	static TestFunction instance;
	return &instance;
}

void TestFunction::Initialize() {}

void TestFunction::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Test Functions");
	ImGui::Checkbox("Vision Cone Color Change", &isVisionConeColorChange_);
	ImGui::Checkbox("Vision Cone Ray Col", &isVisionConeRayCol_);
	ImGui::Checkbox("Player Blood Decal", &isPlayerBloodDecal_);
	ImGui::End();
#endif
}