#pragma once
#include "BaseScene.h"
#include "GameObjects/ResultUI/ResultUIManager.h"
#include "Skybox.h"

/// <summary>
/// リザルトシーン
/// </summary>
class ResultScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// UI管理インスタンス
	std::unique_ptr<ResultUIManager> uiManager_;

	// スカイボックス
	std::unique_ptr<TinyEngine::Skybox> skybox_;

	// メインカメラ
	std::unique_ptr<Camera> mainCamera_;
};
