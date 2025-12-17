#pragma once
#include "BaseScene.h"
#include "Object3d.h"
#include "Sprite.h"
#include "TitleText.h"
#include "StartModel.h"
#include "EndModel.h"
#include <array>
#include <memory>

class TitleScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// 必要なモデルの読み込み
	void AllModelLoad();

	// シーン切り替え処理
	void ChangeScene();

private:
	enum class TitleState { START = 0, END = 1 };

private:
	// タイトルテキストモデル
	std::unique_ptr<TitleText> titleText_;

	// はじめるモデル
	std::unique_ptr<StartModel> startModel_;

	// おわるモデル
	std::unique_ptr<EndModel> endModel_;

	// タイトルの状態
	TitleState titleState_ = TitleState::START;
};
