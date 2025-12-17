#include "StartModel.h"
#include <numbers>

void StartModel::Initialize(EngineContext* ctx) {
	startModel_ = std::make_unique<Object3d>();
	startModel_->Initialize(ctx);
	startModel_->SetModel("start.obj");
	startModel_->SetTranslate({20.0f, 2.0f, 0.0f});
	startModel_->SetScale({5.0f, 5.0f, 5.0f});
	startModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void StartModel::Update(float deltaTime) {
	(void)deltaTime;

	startModel_->Update();
}

void StartModel::Draw() { startModel_->Draw(); }