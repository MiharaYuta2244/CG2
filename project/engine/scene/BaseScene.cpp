#include "BaseScene.h"
#include "SceneManager.h"

void BaseScene::RequestSceneChange(const std::string& sceneName) {
	if (ctx_.sceneManager) {
		ctx_.sceneManager->SetRequestedSceneName(sceneName);
	}
}
