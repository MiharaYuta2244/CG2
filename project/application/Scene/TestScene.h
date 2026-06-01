#pragma once
#include "BaseScene.h"
#include "GameObjects/ObjectRender/ObjectRender.h"

class TestScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// オブジェクトデータの読み込み
	void LoadScene();

private:
	// オブジェクトのリスト
	std::vector<std::unique_ptr<ObjectRender>> sceneObjects;

	std::unique_ptr<ObjectRender> plane_;

	std::unique_ptr<ObjectRender> walk_;
	std::unique_ptr<ObjectRender> sneakWalk_;

	Transform walkTransform_{};
	Transform sneakWalkTransform_{};
};
