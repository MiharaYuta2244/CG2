#pragma once
#pragma once
#include "BaseScene.h"
#include "Menu/MenuList.h"

class TitleScene : public BaseScene{
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* debugCamera, TimeManager* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// メニュー
	std::unique_ptr<MenuList> menu_;
};
