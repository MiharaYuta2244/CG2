#include "Game.h"
#include "Scene/GamePlayScene.h"
#include "Scene/ResultScene.h"
#include "Scene/TitleScene.h"
#include <algorithm>
#include <numbers>

void Game::Initialize() {
	// 基底クラスの初期化処理
	Framework::Initialize();

	// シーンマネージャー初期化
	sceneManager_->Initialize(&GetEngineContext(), GetKeyboard(), GetGamePad(), GetDebugCamera(), GetTimeManager());

	// シーン追加
	sceneManager_->AddScene("Title", std::make_unique<TitleScene>());
	sceneManager_->AddScene("GamePlay", std::make_unique<GamePlayScene>());
	sceneManager_->AddScene("Result", std::make_unique<ResultScene>());

	// 最初のシーンを初期化
	sceneManager_->ChangeScene("Title");

	// フェード用スプライト初期化
	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize(&GetEngineContext(), "resources/white.png");
	fadeSprite_->SetPosition({0.0f, 0.0f});
	fadeSprite_->SetSize({static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight)});
	fadeSprite_->SetAnchorPoint({0.0f, 0.0f});
	fadeSprite_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});
	fadeState_ = FadeState::None;
	fadeTimer_ = 0.0f;
	lastSceneName_ = "";
}

void Game::Update() {
	// 基底クラスの更新処理
	Framework::Update();

	if (sceneManager_->GetRequestedSceneName() != "") {
		requestedSceneName_ = sceneManager_->GetRequestedSceneName();
		sceneManager_->SetRequestedSceneName("");
	}

	if (requestedSceneName_ != "" && fadeState_ == FadeState::None) {
		// シーン切り替え要求があり、フェード中でなければフェードアウト開始
		fadeState_ = FadeState::FadeOut;
		fadeTimer_ = 0.0f;
	}

	// フェード更新（優先して処理）
	if (fadeState_ != FadeState::None) {
		float dt = GetTimeManager()->GetDeltaTime();
		fadeTimer_ += dt;
		float t = fadeDuration_ > 0.0f ? std::clamp(fadeTimer_ / fadeDuration_, 0.0f, 1.0f) : 1.0f;

		if (fadeState_ == FadeState::FadeOut) {
			// フェードアウト進行（透明->不透明）
			fadeSprite_->SetColor({0.0f, 0.0f, 0.0f, t});
			if (t >= 1.0f) {
				// フェードアウト完了 → シーン切り替え
				fadeState_ = FadeState::WaitingForSceneChange;
				fadeTimer_ = 0.0f;
				isSceneChangeRequested_ = true;
			}
		} else if (fadeState_ == FadeState::WaitingForSceneChange) {
			// シーン切り替えを実行
			if (isSceneChangeRequested_) {
				sceneManager_->ChangeScene(requestedSceneName_);
				isSceneChangeRequested_ = false;
				requestedSceneName_ = "";
			}
			// シーン切り替え完了後、フェードインへ移行
			const std::string& currentSceneName = sceneManager_->GetCurrentSceneName();
			if (lastSceneName_ != currentSceneName) {
				lastSceneName_ = currentSceneName;
				fadeState_ = FadeState::FadeIn;
				fadeTimer_ = 0.0f;
			}
		} else if (fadeState_ == FadeState::FadeIn) {
			// フェードイン進行（不透明->透明）
			fadeSprite_->SetColor({0.0f, 0.0f, 0.0f, 1.0f - t});
			if (t >= 1.0f) {
				// フェード完了
				fadeState_ = FadeState::None;
				fadeTimer_ = 0.0f;
				fadeSprite_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});
			}
		}
	}

	// シーンマネージャー更新
	sceneManager_->Update();
}

void Game::Draw() {
	// 描画前処理
	Framework::PreDraw();

	// シーン描画
	sceneManager_->Draw();

	// フェードスプライトを上書きで描画
	if (fadeSprite_) {
		fadeSprite_->Update();
		fadeSprite_->Draw();
	}

	// 描画後処理
	Framework::PostDraw();
}

void Game::Finalize() {
	// シーンマネージャー終了処理
	sceneManager_->Finalize();

	// 基底クラスの終了処理
	Framework::Finalize();
}