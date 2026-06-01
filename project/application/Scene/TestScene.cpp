#include "TestScene.h"
#include "LevelDataLoader.h"
#include "SceneManager.h"

using namespace TinyEngine;

void TestScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;

	// オブジェクトデータの読み込み
	LoadScene();

	plane_ = std::make_unique<ObjectRender>();
	plane_->Initialize(ctx.engineContext, "plane.obj");

	walk_ = std::make_unique<ObjectRender>();
	walk_->Initialize(ctx.engineContext, "walk.gltf");

	sneakWalk_ = std::make_unique<ObjectRender>();
	sneakWalk_->Initialize(ctx.engineContext, "sneakWalk.gltf");

	walkTransform_.scale = {500, 500, 500};
	walkTransform_.rotate = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float>, 0};
	walkTransform_.translate = {-10, 0, 0};

	sneakWalkTransform_.scale = {500, 500, 500};
	sneakWalkTransform_.rotate = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float>, 0};
	sneakWalkTransform_.translate = {0, 0, 0};
}

void TestScene::Update() {
	/*for (auto& object : sceneObjects) {
	    object->Update();
	}*/

	// plane_->Update(transform_);
	walk_->Update(walkTransform_);
	sneakWalk_->Update(sneakWalkTransform_);

#ifdef USE_IMGUI
	ImGui::Begin("transform");
	ImGui::DragFloat3("walkScale", &walkTransform_.scale.x, 1.0f);
	ImGui::DragFloat3("walkRot", &walkTransform_.rotate.x, 0.01f);
	ImGui::DragFloat3("walkPos", &walkTransform_.translate.x, 0.01f);

	ImGui::DragFloat3("SneakWalkScale", &sneakWalkTransform_.scale.x, 1.0f);
	ImGui::DragFloat3("SneakWalkRot", &sneakWalkTransform_.rotate.x, 0.01f);
	ImGui::DragFloat3("SneakWalkPos", &sneakWalkTransform_.translate.x, 0.01f);
	ImGui::End();
#endif
}

void TestScene::Draw() {
	/*for (auto& object : sceneObjects) {
	    object->Draw();
	}*/

	// plane_->Draw();
	walk_->Draw();
	sneakWalk_->Draw();
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
