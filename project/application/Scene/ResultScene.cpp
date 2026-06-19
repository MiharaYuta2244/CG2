#include "ResultScene.h"
#include "SceneManager.h"

using namespace TinyEngine;

void ResultScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();

	// UI管理クラス生成&初期化
	uiManager_ = std::make_unique<ResultUIManager>();
	uiManager_->Initialize(ctx.engineContext, commonData_);

	// コールバック登録
	uiManager_->GetRetryButton()->SetCallback([this]() { RequestSceneChange("GamePlay"); });
	uiManager_->GetToTitleButton()->SetCallback([this]() { RequestSceneChange("Title"); });

	// 生成したボタンをリストに登録する
	uiManager_->RegisterButtons();

	// メインカメラ
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize();
}

void ResultScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

	// UI管理クラス更新
	uiManager_->Update(deltaTime, ctx_.keyboard, ctx_.gamePad);
}

void ResultScene::Draw() {
	// UI管理クラス描画
	uiManager_->Draw();
}

void ResultScene::Finalize() {}
