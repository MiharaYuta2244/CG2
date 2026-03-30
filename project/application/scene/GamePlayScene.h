#pragma once
#include "BaseScene.h"
#include "Object3d.h"
#include "Particle.h"
#include "Sprite.h"
#include "XAudio.h"
#include <array>
#include <memory>
#include <vector>

class GamePlayScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;
private:

};
