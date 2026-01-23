#pragma once
#include "BaseScene.h"
#include "Object3d.h"
#include "Sprite.h"
#include <array>
#include <memory>

class TestScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// 球モデル
	std::unique_ptr<Object3d> sphereModel_;

	// Plane.obj
	std::unique_ptr<Object3d> planeObjModel_;

	// Plane.glb
	std::unique_ptr<Object3d> planeGltfModel_;
};
