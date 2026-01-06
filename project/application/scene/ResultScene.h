#pragma once
#include "BaseScene.h"
#include "Object3d.h"
#include "ReStartModel.h"
#include "ToTitleModel.h"
#include "BothCurtain.h"
#include "ResultModel.h"
#include "XAudio.h"
#include <memory>

class ResultScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// リザルト文字列を読み込み
	std::string LoadResultStatus();

private:
	// さいかいモデル
	std::unique_ptr<ReStartModel> restartModel_;

	// たいとるへモデル
	std::unique_ptr<ToTitleModel> toTitleModel_;

	// 画面両端の幕
	std::unique_ptr<BothCurtain> rightCurtain_;
	std::unique_ptr<BothCurtain> leftCurtain_;

	// リザルトモデル
	std::unique_ptr<ResultModel> resultModel_;

	std::unique_ptr<XAudio> audio_;
};
