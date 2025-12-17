#include "TitleScene.h"
#include "Easing.h"
#include "SceneManager.h"
#include <algorithm>
#include <numbers>

void TitleScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	// モデルの読み込み
	AllModelLoad();

	// タイトルテキストモデル
	titleText_ = std::make_unique<TitleText>();
	titleText_->Initialize(engineContext_);

	// はじめるモデル
	startModel_ = std::make_unique<StartModel>();
	startModel_->Initialize(engineContext_);

	// おわるモデル
	endModel_ = std::make_unique<EndModel>();
	endModel_->Initialize(engineContext_);

	// タイトルの状態
	titleState_ = TitleState::START;
}

void TitleScene::Update() {
	// モデルの更新
	titleText_->Update(timeManager_->GetDeltaTime());
	startModel_->Update(timeManager_->GetDeltaTime());
	endModel_->Update(timeManager_->GetDeltaTime());

	// シーン切り替え処理
	ChangeScene();
}

void TitleScene::Draw() {
	titleText_->Draw();
	startModel_->Draw();
	endModel_->Draw();
}

void TitleScene::Finalize() {
	titleText_.reset();
	startModel_.reset();
	endModel_.reset();
}

void TitleScene::AllModelLoad() {
	engineContext_->modelManager->LoadModel("fence.obj");
	engineContext_->modelManager->LoadModel("plane.obj");
	engineContext_->modelManager->LoadModel("axis.obj");
	engineContext_->modelManager->LoadModel("SkySphere.obj");
	engineContext_->modelManager->LoadModel("skydome.obj");
	engineContext_->modelManager->LoadModel("Field.obj");
	engineContext_->modelManager->LoadModel("sphere.obj");
	engineContext_->modelManager->LoadModel("Box.obj");
	engineContext_->modelManager->LoadModel("Boss.obj");
	engineContext_->modelManager->LoadModel("Hiyoko.obj");
	engineContext_->modelManager->LoadModel("HiyokoGlass.obj");
	engineContext_->modelManager->LoadModel("HiyokoStudent.obj");
	engineContext_->modelManager->LoadModel("HiyokoAfro.obj");
	engineContext_->modelManager->LoadModel("Title.obj");
	engineContext_->modelManager->LoadModel("HiyokoPropeller.obj");
	engineContext_->modelManager->LoadModel("ClimbDrop.obj");
	engineContext_->modelManager->LoadModel("tree.obj");
	engineContext_->modelManager->LoadModel("saikai.obj");
	engineContext_->modelManager->LoadModel("ToTitle2.obj");
	engineContext_->modelManager->LoadModel("start.obj");
	engineContext_->modelManager->LoadModel("end.obj");
	engineContext_->modelManager->LoadModel("grape.obj");
}

void TitleScene::ChangeScene() {
	// 入力
	bool upInput = keyboard_->KeyTriggered(DIK_W) || gamePad_->GetState().buttonsPressed.dpadUp;
	bool downInput = keyboard_->KeyTriggered(DIK_S) || gamePad_->GetState().buttonsPressed.dpadDown;

	// 上入力の場合
	if (upInput) {
		titleState_ = TitleState::START; // ゲーム開始
		startModel_->SetSelected(true);  // 選択状態にする
		endModel_->SetSelected(false);   // 選択状態を解除
	}

	// 下入力の場合
	if (downInput) {
		titleState_ = TitleState::END;   // ゲーム終了
		endModel_->SetSelected(true);    // 選択状態にする
		startModel_->SetSelected(false); // 選択状態を解除
	}

	// ゲーム開始入力
	bool startInput = keyboard_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;
	if (startInput && titleState_ == TitleState::START) {
		sceneManager_->ChangeScene("GamePlay"); // ゲームシーンへ
	} else if (startInput && titleState_ == TitleState::END) {
		PostQuitMessage(0); // ゲーム終了
	}
}
