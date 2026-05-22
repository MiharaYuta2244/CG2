#include "Game.h"
#include "Scene/EasingEditorScene.h"
#include "Scene/GamePlayScene.h"
#include "Scene/TitleScene.h"
#include "Scene/PauseScene.h"
#include "Scene/ResultScene.h"

using namespace TinyEngine;

void Game::Initialize() {
	// 基底クラスの初期化処理
	Framework::Initialize();

	// シーンマネージャーの生成&初期化
	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(&GetEngineContext(), GetKeyboard(), GetGamePad(), GetDebugCamera(), GetTimeManager());

	// シーン追加
	sceneManager_->AddScene("Title", std::make_unique<TitleScene>());
	sceneManager_->AddScene("GamePlay", std::make_unique<GamePlayScene>());
	sceneManager_->AddScene("EasingEditorScene", std::make_unique<EasingEditorScene>());
	sceneManager_->AddScene("Pause", std::make_unique<PauseScene>());
	sceneManager_->AddScene("Result", std::make_unique<ResultScene>());

	// 最初のシーンを初期化
	sceneManager_->ChangeScene("Title");

	// フェードマネージャーの生成&初期化
	fadeManager_ = std::make_unique<FadeManager>();
	fadeManager_->Initialize(&GetEngineContext());
}

void Game::Update() {
	// 基底クラスの更新処理
	Framework::Update();

	// シーン切り替えの要求があればフェードアウト開始
	if (sceneManager_->GetRequestedSceneName() != "" && sceneManager_->GetRequestedTransition() == SceneTransition::Change) {
		fadeManager_->FadeOutTo(sceneManager_->GetRequestedSceneName());
		sceneManager_->ClearRequest();
	}

	// フェードの更新処理
	float dt = GetTimeManager()->GetDeltaTime();
	fadeManager_->Update(dt);

	// 画面が完全に暗くなったらシーンを切り替える
	if (fadeManager_->IsWaitingForSceneShange()) {
		sceneManager_->ChangeScene(fadeManager_->GetRequestedSceneName());
		fadeManager_->NotifySceneChanged();
	}

	// シーンマネージャー更新
	sceneManager_->Update();
}

void Game::Draw() {
	Framework::BeginRender();

	Framework::SRVManagerPreDraw();

	// シーン描画
	sceneManager_->Draw();

	// フェードスプライトを上書きで描画
	fadeManager_->Draw();

	Framework::EndRender();

	// 描画前処理
	Framework::PreDraw();

	// 描画後処理
	Framework::PostDraw();
}

void Game::Finalize() {
	// シーンマネージャー終了処理
	sceneManager_->Finalize();

	// 基底クラスの終了処理
	Framework::Finalize();
}