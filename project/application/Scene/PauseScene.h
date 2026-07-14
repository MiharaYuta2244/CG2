#pragma once
#include "BaseScene.h"
#include "Sprite.h"
#include "Menu/MenuList.h"

/// <summary>
/// ポーズシーン
/// </summary>
class PauseScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	std::unique_ptr<MenuList> menuList_;
	std::unique_ptr<TinyEngine::Sprite> bgSprite_;
};
