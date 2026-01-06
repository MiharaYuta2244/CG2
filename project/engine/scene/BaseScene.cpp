#include "BaseScene.h"
#include "SceneManager.h"

void BaseScene::RequestSceneChange(const std::string& sceneName) {
	if (sceneManager_) {
		sceneManager_->SetRequestedSceneName(sceneName);
	}
}
