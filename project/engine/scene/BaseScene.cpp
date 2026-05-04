#include "BaseScene.h"
#include "SceneManager.h"

void BaseScene::RequestSceneChange(const std::string& sceneName) {
	if (ctx_.sceneManager) {
		ctx_.sceneManager->SetRequestedSceneName(sceneName);
		ctx_.sceneManager->ChangeScene(sceneName);
	}
}

void BaseScene::RequestScenePush(const std::string& sceneName) {
	if (ctx_.sceneManager) {
		ctx_.sceneManager->SetRequestedSceneName(sceneName);
		ctx_.sceneManager->PushScene(sceneName);
	}
}

void BaseScene::RequestScenePop() {
	if (ctx_.sceneManager) {
		ctx_.sceneManager->PopScene();
	}
}
