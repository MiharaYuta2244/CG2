#pragma once
#pragma once
#include "BaseScene.h"
#include "Menu/MenuList.h"

class TitleScene : public BaseScene{
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// メニュー
	std::unique_ptr<MenuList> menu_;
};
