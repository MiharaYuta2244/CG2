#include "ResultScene.h"
#include "SceneManager.h"
#include <numbers>
#include <fstream>

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

	// 画面両端の幕
	rightCurtain_ = std::make_unique<BothCurtain>();
	rightCurtain_->Initialize(ctx);
	leftCurtain_ = std::make_unique<BothCurtain>();
	leftCurtain_->Initialize(ctx);

	// 結果ステータスをファイルから読み込む
	std::string resultStatus = LoadResultStatus();

	// リザルトモデル
	resultModel_ = std::make_unique<ResultModel>();
	resultModel_->Initialize(engineContext_, resultStatus);
}

void ResultScene::Update() {
	restartModel_->Update(timeManager_->GetDeltaTime());
	toTitleModel_->Update(timeManager_->GetDeltaTime());

	rightCurtain_->Update();
	leftCurtain_->Update();

	// リザルトモデル
	resultModel_->Update(timeManager_->GetDeltaTime());

	bool rightInput = keyboard_->KeyTriggered(DIK_D) || gamePad_->GetState().buttonsPressed.dpadRight;
	bool leftInput = keyboard_->KeyTriggered(DIK_A) || gamePad_->GetState().buttonsPressed.dpadLeft;
	bool decideInput = keyboard_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;

	if (rightInput) {
		toTitleModel_->SetSelected(true);
		restartModel_->SetSelected(false);
	}

	if (leftInput) {
		restartModel_->SetSelected(true);
		toTitleModel_->SetSelected(false);
	}

	if (decideInput && toTitleModel_->GetSelected()) {
		RequestSceneChange("Title");
	}

	if (decideInput && restartModel_->GetSelected()) {
		RequestSceneChange("GamePlay");
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

	rightCurtain_->Draw();
	leftCurtain_->Draw();

	// リザルトモデル
	resultModel_->Draw();
}

void ResultScene::Finalize() {
	restartModel_.reset();
	toTitleModel_.reset();
}

std::string ResultScene::LoadResultStatus() {
	std::ifstream file("resources/result_status.txt");
	std::string status;
	if (file.is_open()) {
		std::getline(file, status);
		file.close();
	}
	return status;
}
