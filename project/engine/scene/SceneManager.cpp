#include "SceneManager.h"

void SceneManager::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* debugCamera, TimeManager* timeManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
}

void SceneManager::AddScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene) {
	scenes_[sceneName] = std::move(scene);
}

void SceneManager::ChangeScene(const std::string& sceneName) {
	// シーンが存在するか確認
	if (scenes_.find(sceneName) == scenes_.end()) {
		return;
	}

	// 既に切り替え中なら無視
	if (isChangingScene_) {
		return;
	}

	// シーン切り替え要求を記録
	nextSceneName_ = sceneName;
	isChangingScene_ = true;
}

void SceneManager::Update() {
	// シーン切り替え処理
	if (isChangingScene_ && nextSceneName_ != currentSceneName_) {
		// 現在のシーンを終了
		if (currentScene_) {
			currentScene_->Finalize();
		}

		// シーン切り替え
		currentSceneName_ = nextSceneName_;
		currentScene_ = scenes_[currentSceneName_].get();

		// シーンに必要なデータをまとめて渡す
		SceneContext ctx;
		ctx.engineContext = engineContext_;
		ctx.keyboard = keyboard_;
		ctx.gamePad = gamePad_;
		ctx.currentCamera = debugCamera_;
		ctx.timeManager = timeManager_;
		ctx.sceneManager = this;

		// 新しいシーンを初期化
		currentScene_->Initialize(ctx);

		isChangingScene_ = false;
	}

	// 現在のシーンを更新
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::Draw() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::Finalize() {
	if (currentScene_) {
		currentScene_->Finalize();
	}

	for (auto& scene : scenes_) {
		if (scene.second) {
			scene.second->Finalize();
		}
	}

	scenes_.clear();
	currentScene_ = nullptr;
}