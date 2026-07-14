#include "PauseScene.h"

using namespace TinyEngine;

void PauseScene::Initialize(const SceneContext& ctx) { 
	ctx_ = ctx;

	// メニューの生成&初期化
	menuList_ = std::make_unique<MenuList>();
	menuList_->Initialize(ctx.engineContext);

	// メニュー項目の追加
	menuList_->AddItem("Resume", "white.png", [this]() { RequestScenePop(); });
	menuList_->AddItem("Option", "Title_Option.png", [this]() { RequestSceneChange("Option"); });
	menuList_->AddItem("Title", "white.png", [this]() { RequestSceneChange("Title"); });
}

void PauseScene::Update() {
	if (ctx_.keyboard->KeyTriggered(DIK_TAB) || ctx_.gamePad->GetState().buttonsPressed.start) {
		RequestScenePop();
	}

	// メニューの更新 
	menuList_->Update(ctx_.keyboard, ctx_.gamePad, ctx_.timeManager->GetDeltaTime());

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

void PauseScene::Draw() {
	// メニューの描画
	menuList_->Draw();
}

void PauseScene::Finalize() {}
