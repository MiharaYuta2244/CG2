#include "TitleText.h"
#include "Easing.h"
#include <algorithm>
#include <numbers>

void TitleText::Initialize(EngineContext* ctx) {
	textModel_ = std::make_unique<Object3d>();
	textModel_->Initialize(ctx);
	textModel_->SetModel("ClimbDrop.obj");
	textModel_->SetTranslate({21.68f, 13.39f, 12.76f});
	textModel_->SetRotate({-0.72f, 2.612f, 0.2f});
	textModel_->SetScale({15.0f, 15.0f, 6.0f});
	textModel_->SetEnableLighting(true);
	textModel_->GetDirectionalLight().direction = {12.0f, 3.8f, 12.0f};
	textModel_->GetDirectionalLight().intensity = 0.05f;
	textModel_->GetMaterial().shininess = 64.0f;
	textModel_->SetColor({1.0f, 0.7f, 0.0f, 1.0f});
}

void TitleText::Update(float deltaTime) {
	(void)deltaTime;

	textModel_->Update();
}

void TitleText::Draw() { textModel_->Draw(); }