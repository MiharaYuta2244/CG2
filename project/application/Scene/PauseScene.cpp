#include "PauseScene.h"

void PauseScene::Initialize(const SceneContext& ctx) { ctx_ = ctx; }

void PauseScene::Update() {
	if (ctx_.keyboard->KeyTriggered(DIK_TAB)) {
		RequestScenePop();
	}

	#ifdef USE_IMGUI
	ImGui::Begin("Pause Menu");

	// 音量調整
	static float volume = 1.0f;
	if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f)) {
		// サウンドマネージャー等に音量をセットする
	}

	// ゲームに戻る
	if (ImGui::Button("Resume")) {
		RequestScenePop();
	}

	// タイトルに戻る
	if (ImGui::Button("Back to Title")) {
		RequestSceneChange("Title");
	}

	ImGui::End();
#endif
}

void PauseScene::Draw() {}

void PauseScene::Finalize() {}
