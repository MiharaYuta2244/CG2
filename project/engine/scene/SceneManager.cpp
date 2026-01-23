#include "SceneManager.h"

void SceneManager::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;

	// モデルの読み込み
	AllModelLoad();
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

		// 新しいシーンを初期化
		currentScene_->Initialize(engineContext_, keyboard_, gamePad_, debugCamera_, timeManager_, this);

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

void SceneManager::AllModelLoad() {
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
	engineContext_->modelManager->LoadModel("HiyokoPropeller.obj");
	engineContext_->modelManager->LoadModel("ClimbDrop.obj");
	engineContext_->modelManager->LoadModel("tree.obj");
	engineContext_->modelManager->LoadModel("saikai.obj");
	engineContext_->modelManager->LoadModel("ToTitle2.obj");
	engineContext_->modelManager->LoadModel("start.obj");
	engineContext_->modelManager->LoadModel("end.obj");
	engineContext_->modelManager->LoadModel("grape.obj");
	engineContext_->modelManager->LoadModel("apple.obj");
	engineContext_->modelManager->LoadModel("orange.obj");
	engineContext_->modelManager->LoadModel("return.obj");
	engineContext_->modelManager->LoadModel("stage1.obj");
	engineContext_->modelManager->LoadModel("stage2.obj");
	engineContext_->modelManager->LoadModel("stage3.obj");
	engineContext_->modelManager->LoadModel("charaSelect.obj");
	engineContext_->modelManager->LoadModel("Cloud2.obj");
	engineContext_->modelManager->LoadModel("StageClear.obj");
	engineContext_->modelManager->LoadModel("GameOver2.obj");
	engineContext_->modelManager->LoadModel("field1.obj");
	engineContext_->modelManager->LoadModel("enemy.obj");
	engineContext_->modelManager->LoadModel("AirplaneLow.glb");
	engineContext_->modelManager->LoadModel("Cube.obj");
	engineContext_->modelManager->LoadModel("TurretBullet.obj");
	engineContext_->modelManager->LoadModel("GunTurret.obj");
	engineContext_->modelManager->LoadModel("MonsterBall.obj");
	engineContext_->modelManager->LoadModel("plane.gltf");
}