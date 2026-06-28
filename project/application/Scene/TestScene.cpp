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

	KeyframeAnimation keyframeAnimation;

	walk_ = std::make_unique<ObjectRender>();
	walk_->Initialize(ctx.engineContext, "walk.gltf");
	Animation walkAnimation = keyframeAnimation.LoadAnimationFile("walk.gltf");
	walk_->GetObject3d()->PlayAnimation(walkAnimation);
	walk_->SetIsSkinning(true);

	sneakWalk_ = std::make_unique<ObjectRender>();
	sneakWalk_->Initialize(ctx.engineContext, "sneakWalk.gltf");
	Animation sneakWalkAnimation = keyframeAnimation.LoadAnimationFile("sneakWalk.gltf");
	sneakWalk_->GetObject3d()->PlayAnimation(sneakWalkAnimation);
	sneakWalk_->SetIsSkinning(true);

	testObj_ = std::make_unique<ObjectRender>();
	testObj_->Initialize(ctx.engineContext, "Hiyoko.obj");

	walkTransform_.scale = {500, 500, 500};
	walkTransform_.rotate = {0, 0, 0};
	walkTransform_.translate = {-10, 0, 0};

	sneakWalkTransform_.scale = {500, 500, 500};
	sneakWalkTransform_.rotate = {0, 0, 0};
	sneakWalkTransform_.translate = {0, 0, 0};
}

void TestScene::Update() {
	for (auto& object : sceneObjects) {
	    object->Update();
	}

	// plane_->Update(transform_);
	walk_->Update(walkTransform_);
	sneakWalk_->Update(sneakWalkTransform_);
	testObj_->Update();

	if(ctx_.keyboard->KeyTriggered(DIK_SPACE)){
		std::unique_ptr<TinyEngine::Particle> particle = std::make_unique<TinyEngine::Particle>();

		// UVスクロール速度とエフェクトの色
		Vector2 scrollSpeed = {1.0f, 0.0f};
		Vector4 effectColor = {0.0f, 1.0f, 1.0f, 1.0f};

		// モジュールの生成 (自分自身のポインタを渡す)
		auto module = std::make_unique<UVScrollModule>(scrollSpeed, effectColor);

		// 発生位置を設定
		Vector3 spawnPos = {0.0f, 0.0f, 0.0f};

		// 初期化にモジュールとカスタムエミッタを渡す
		particle->Initialize(ctx_.engineContext, spawnPos, "gradationLine.png", std::move(module), nullptr, TinyEngine::ParticleMeshType::Cylinder);

		particle->SetAlphaCutoff(0.5f);

		// ループ発生を無効にする
		particle->SetEmitMode(false);

		particles_.push_back(std::move(particle));
	}

	for(auto& particle : particles_){
		particle->Update();
	}

	std::erase_if(particles_, [this](const std::unique_ptr<TinyEngine::Particle>& p) {
		return p->IsFinished();
	});

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
	testObj_->Draw();

	/*for (auto& particle : particles_) {
		particle->Draw();
	}*/
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
