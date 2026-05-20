#include "ResultScene.h"

using namespace TinyEngine;

void ResultScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;

	// UI管理クラス生成&初期化
	uiManager_ = std::make_unique<ResultUIManager>();
	uiManager_->Initialize(ctx.engineContext);

	// コールバック登録
	uiManager_->GetRetryButton()->SetCallback([this]() { RequestSceneChange("GamePlay"); });
	uiManager_->GetToTitleButton()->SetCallback([this]() { RequestSceneChange("Title"); });

	// 生成したボタンをリストに登録する
	uiManager_->RegisterButtons();

	// スカイボックスの生成&初期化
	skybox_ = std::make_unique<Skybox>();
	skybox_->Initialize(ctx_.engineContext, "rostock_laage_airport_4k.dds");

	// メインカメラ
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize();

	// AnimationCube
	animationCube_=std::make_unique<Object3d>();
	animationCube_->Initialize(ctx.engineContext);
}

void ResultScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

#ifdef USE_IMGUI

	Vector3 position = skybox_->GetPosition();
	Vector3 scale = skybox_->GetScale();

	ImGui::Begin("Skybox");
	ImGui::DragFloat3("Position", &position.x, 0.01f);
	ImGui::DragFloat3("Scale", &scale.x, 0.01f);
	ImGui::End();

	skybox_->SetPosition(position);
	skybox_->SetScale(scale);

#endif

	// スカイボックス更新
	skybox_->Update(mainCamera_->GetViewMatrix(), mainCamera_->GetProjection());

	// UI管理クラス更新
	uiManager_->Update(deltaTime, ctx_.keyboard);
}

void ResultScene::Draw() {
	// スカイボックス描画
	//skybox_->Draw();

	// UI管理クラス描画
	uiManager_->Draw();
}

void ResultScene::Finalize() {}
