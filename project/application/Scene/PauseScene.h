#pragma once
#include "BaseScene.h"

class PauseScene : public BaseScene{
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;
};
