#pragma once
#ifdef  USE_IMGUI
#include "ImGuiManager.h"
#endif //  USE_IMGUI


/// <summary>
/// テスト機能
/// </summary>
class TestFunction {
public:
	static TestFunction* GetInstance();

	void Initialize();

	void DrawImGui();

	bool GetIsVisionConeColorChange() const { return isVisionConeColorChange_; }
	bool GetIsVisionConeRayCol() const { return isVisionConeRayCol_; }
	bool GetIsPlayerBloodDecal() const { return isPlayerBloodDecal_; }

private:
	TestFunction() = default;
	~TestFunction() = default;
	TestFunction(const TestFunction&) = delete;
	TestFunction& operator=(const TestFunction&) = delete;

private:
	bool isVisionConeColorChange_ = true;
	bool isVisionConeRayCol_ = true;
	bool isPlayerBloodDecal_ = true;
};
