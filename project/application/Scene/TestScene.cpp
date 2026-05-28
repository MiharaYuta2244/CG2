#include "TestScene.h"
#include "LevelDataLoader.h"
#include "SceneManager.h"

using namespace TinyEngine;

void TestScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;

	// オブジェクトデータの読み込み
	LoadScene();

	plane_=std::make_unique<ObjectRender>();
	plane_->Initialize(ctx.engineContext, "plane.obj");
}

void TestScene::Update() {
	for (auto& object : sceneObjects) {
		object->Update();
	}
}

void TestScene::Draw() {
	for (auto& object : sceneObjects) {
		object->Draw();
	}
}

void TestScene::Finalize() {}

void TestScene::LoadScene() {
	LevelData level;
	if (!LevelDataLoader::LoadLevel("test.json", level)) {
		return;
	}

	for (auto& obj : level.objects) {
		// オブジェクトのタイプがMESHの時だけモデルを生成
		if (obj.type == "MESH") {
			auto render = std::make_unique<ObjectRender>();
			render->Initialize(ctx_.engineContext, obj.name + ".obj");

			Transform t;
			t.translate = obj.transform.translate;
			t.rotate = MathUtility::DegreeToRadian(obj.transform.rotate);
			t.scale = obj.transform.scale;

			render->SetTransform(t);

			sceneObjects.push_back(std::move(render));
		}
	}
}
