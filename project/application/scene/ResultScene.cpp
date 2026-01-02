#include "ResultScene.h"
#include "SceneManager.h"
#include <numbers>

void ResultScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	// さいかいモデル
	restartModel_ = std::make_unique<ReStartModel>();
	restartModel_->Initialize(engineContext_);

	// たいとるへモデル
	toTitleModel_ = std::make_unique<ToTitleModel>();
	toTitleModel_->Initialize(engineContext_);
}

void ResultScene::Update() {
	restartModel_->Update(timeManager_->GetDeltaTime());
	toTitleModel_->Update(timeManager_->GetDeltaTime());

	bool backInput = keyboard_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;
	if (backInput) {
		sceneManager_->ChangeScene("Title");
	}

#ifdef USE_IMGUI
	ImGui::Begin("Result");
	ImGui::TextWrapped("Result Scene");
	ImGui::TextWrapped("Press Space or GamePad A to Return to Title");
	ImGui::End();
#endif
}

void ResultScene::Draw() {
	restartModel_->Draw();
	toTitleModel_->Draw();
}

void ResultScene::Finalize() {
	restartModel_.reset();
	toTitleModel_.reset();
}
