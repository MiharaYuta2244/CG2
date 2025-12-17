#include "EndModel.h"
#include <numbers>

void EndModel::Initialize(EngineContext* ctx){
	endModel_ = std::make_unique<Object3d>();
	endModel_->Initialize(ctx);
	endModel_->SetModel("end.obj");
	endModel_->SetTranslate({20.0f, -2.0f, 0.0f});
	endModel_->SetScale({5.0f, 5.0f, 5.0f});
	endModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void EndModel::Update(float deltaTime){
	(void)deltaTime;

	endModel_->Update();
}

void EndModel::Draw(){
	endModel_->Draw();
}