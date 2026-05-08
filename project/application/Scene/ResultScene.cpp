#include "ResultScene.h"

void ResultScene::Initialize(const SceneContext& ctx) { 
	ctx_ = ctx; 

	// UI管理クラス生成&初期化
	uiManager_=std::make_unique<ResultUIManager>();
	uiManager_->Initialize(ctx.engineContext);
}

void ResultScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

	// タイトルへ戻る
	if (ctx_.keyboard->KeyTriggered(DIK_SPACE)) {
		RequestSceneChange("Title");
	}

	// UI管理クラス更新
	uiManager_->Update(deltaTime, ctx_.keyboard);

#ifdef USE_IMGUI
	ImGui::Begin("Result");

	ImGui::End();
#endif
}

void ResultScene::Draw() {
	// UI管理クラス描画
	uiManager_->Draw();
}

void ResultScene::Finalize() {}
