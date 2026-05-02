#include "TitleScene.h"
#include "SceneManager.h"

void TitleScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* debugCamera, TimeManager* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	mainCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	// メニューの生成&初期化
	menu_ = std::make_unique<MenuList>();
	menu_->Initialize(ctx);
	menu_->AddItem("Play", "Title_Play.png", [this]() { RequestSceneChange("GamePlay"); });
	menu_->AddItem("Option", "Title_Option.png", [this]() { RequestSceneChange("GamePlay"); });
	menu_->AddItem("Quit", "Title_Quit.png", [this]() { PostQuitMessage(0); });
}

void TitleScene::Update() {
	// メニューの更新
	menu_->Update(keyboard_, timeManager_->GetDeltaTime());
}

void TitleScene::Draw() {
	// メニューの描画
	menu_->Draw();
}

void TitleScene::Finalize() {}
