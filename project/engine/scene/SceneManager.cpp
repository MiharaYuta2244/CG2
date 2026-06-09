#include "SceneManager.h"

void SceneManager::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* debugCamera, TimeManager* timeManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;

	commonData_ = {};
}

void SceneManager::AddScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene) {
	scenes_[sceneName] = std::move(scene);
}

void SceneManager::ChangeScene(const std::string& sceneName) {
	if (scenes_.find(sceneName) == scenes_.end() || requestedTransition_ != SceneTransition::None)
		return;
	nextSceneName_ = sceneName;
	requestedTransition_ = SceneTransition::Change;
}

void SceneManager::PushScene(const std::string& sceneName) {
	if (scenes_.find(sceneName) == scenes_.end() || requestedTransition_ != SceneTransition::None)
		return;
	nextSceneName_ = sceneName;
	requestedTransition_ = SceneTransition::Push;
}

void SceneManager::PopScene() {
	if (sceneStack_.size() <= 1 || requestedTransition_ != SceneTransition::None)
		return;
	requestedTransition_ = SceneTransition::Pop;
}

BaseScene* SceneManager::GetCurrentScene() const { return sceneStack_.empty() ? nullptr : sceneStack_.back(); }

void SceneManager::Update() {
	// シーン遷移処理
	if (requestedTransition_ != SceneTransition::None) {
		if (requestedTransition_ == SceneTransition::Change) {
			// 全てのシーンを終了してクリア
			for (auto it = sceneStack_.rbegin(); it != sceneStack_.rend(); ++it) {
				(*it)->Finalize();
			}
			sceneStack_.clear();

			// 新しいシーンを追加
			BaseScene* next = scenes_[nextSceneName_].get();
			SceneContext ctx = {engineContext_, keyboard_, gamePad_, debugCamera_, timeManager_, this};
			next->Initialize(ctx);
			sceneStack_.push_back(next);

		} else if (requestedTransition_ == SceneTransition::Push) {
			// 現在のシーンは終了せずに、新しいシーンを上に重ねる
			BaseScene* next = scenes_[nextSceneName_].get();
			SceneContext ctx = {engineContext_, keyboard_, gamePad_, debugCamera_, timeManager_, this};
			next->Initialize(ctx);
			sceneStack_.push_back(next);

		} else if (requestedTransition_ == SceneTransition::Pop) {
			// 一番上のシーンだけ終了して取り除く
			sceneStack_.back()->Finalize();
			sceneStack_.pop_back();
		}
		requestedTransition_ = SceneTransition::None;
	}

	// 現在アクティブのシーンだけ更新する
	if (!sceneStack_.empty()) {
		sceneStack_.back()->Update();
	}
}

void SceneManager::Draw() {
	for (BaseScene* scene : sceneStack_) {
		if (scene) {
			scene->Draw();
		}
	}
}

void SceneManager::Finalize() {
	for (auto it = sceneStack_.rbegin(); it != sceneStack_.rend(); ++it) {
		(*it)->Finalize();
	}
	sceneStack_.clear();
	scenes_.clear();
}