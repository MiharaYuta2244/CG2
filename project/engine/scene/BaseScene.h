#pragma once
#include "SceneContext.h"
#include "GameObjects/CommonData.h"

/// <summary>
/// シーンの基底クラス
/// </summary>
class BaseScene {
public:
	virtual ~BaseScene() = default;

	// シーン初期化
	virtual void Initialize(const SceneContext& ctx) = 0;

	// シーン更新
	virtual void Update() = 0;

	// シーン描画
	virtual void Draw() = 0;

	// シーン終了処理
	virtual void Finalize() = 0;

	virtual void RequestSceneChange(const std::string& sceneName);
	virtual void RequestScenePush(const std::string& sceneName);
	virtual void RequestScenePop();

protected:
	SceneContext ctx_;
	CommonData* commonData_;
};
