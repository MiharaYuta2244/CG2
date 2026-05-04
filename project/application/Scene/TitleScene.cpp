#include "TitleScene.h"
#include "SceneManager.h"

void TitleScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;

	// メニューの生成&初期化
	menu_ = std::make_unique<MenuList>();
	menu_->Initialize(ctx.engineContext);
	menu_->AddItem("Play", "Title_Play.png", [this]() { RequestSceneChange("GamePlay"); });
	menu_->AddItem("Option", "Title_Option.png", [this]() { RequestSceneChange("GamePlay"); });
	menu_->AddItem("Quit", "Title_Quit.png", [this]() { PostQuitMessage(0); });
}

void TitleScene::Update() {
	// メニューの更新
	menu_->Update(ctx_.keyboard, ctx_.timeManager->GetDeltaTime());
}

void TitleScene::Draw() {
	// メニューの描画
	menu_->Draw();
}

void TitleScene::Finalize() {}
