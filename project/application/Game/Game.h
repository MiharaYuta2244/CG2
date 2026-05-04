#pragma once
#include "Framework.h"
#include "SceneManager.h"
#include "Fade/FadeManager.h"
#include <memory>

class Game : public Framework {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// シーンマネージャー
	std::unique_ptr<SceneManager> sceneManager_;

	// フェードマネージャー
	std::unique_ptr<FadeManager> fadeManager_;
};