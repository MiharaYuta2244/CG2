#include "TestScene.h"

void TestScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	debugCamera_->SetTranslation({0.0f, 0.0f, -10.0f});

	// 球モデル生成&初期化
	sphereModel_ = std::make_unique<Object3d>();
	sphereModel_->Initialize(ctx);
	sphereModel_->SetModel("MonsterBall.obj");
	sphereModel_->SetRotate({0.0f, -std::numbers::pi_v<float> / 2, 0.0f});

	// PlaneObj生成&初期化
	planeObjModel_ = std::make_unique<Object3d>();
	planeObjModel_->Initialize(ctx);
	planeObjModel_->SetModel("plane.obj");
	planeObjModel_->SetTranslate({5.0f, 0.0f, 0.0f});
	planeObjModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});

	// PlaneGlb生成&初期化
	planeGltfModel_ = std::make_unique<Object3d>();
	planeGltfModel_->Initialize(ctx);
	planeGltfModel_->SetModel("plane.gltf");
	planeGltfModel_->SetTranslate({-5.0f, 0.0f, 0.0f});
	planeGltfModel_->SetRotate({std::numbers::pi_v<float> / 2, std::numbers::pi_v<float>, 0.0f});
}

void TestScene::Update() {
	sphereModel_->Update();
	planeObjModel_->Update();
	planeGltfModel_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("Light");
	ImGui::DragFloat3("Direction", &sphereModel_->GetDirectionalLight().direction.x, 0.01f);
	ImGui::DragFloat("Intensity", &sphereModel_->GetDirectionalLight().intensity, 0.01f);
	ImGui::ColorEdit4("Color", &sphereModel_->GetDirectionalLight().color.x);
	ImGui::DragFloat("shininess", &sphereModel_->GetMaterial().shininess);
	ImGui::End();
#endif
}

void TestScene::Draw() {
	sphereModel_->Draw();
	planeObjModel_->Draw();
	planeGltfModel_->Draw();
}

void TestScene::Finalize() {}
