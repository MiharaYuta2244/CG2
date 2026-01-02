#include "TitleText.h"
#include "Easing.h"
#include <numbers>
#include <algorithm>

void TitleText::Initialize(EngineContext* ctx) {
	textModel_ = std::make_unique<Object3d>();
	textModel_->Initialize(ctx);
	textModel_->SetModel("ClimbDrop.obj");
	textModel_->SetScale({12.0f, 12.0f, 12.0f});
	float posY = std::numbers::pi_v<float>;
	textModel_->SetRotate({0.0f, posY, 0.0f});
	textModel_->SetTranslate({19.0f, 8.0f, 0.0f});
}

void TitleText::Update(float deltaTime) {
	Vector3 scale = textModel_->GetScale();

	timer_ += deltaTime;
	float duration = 0.5f;

	float t = fmod(timer_, duration) / duration;
	float eased = Easing::easeInOutQuad(t);

	scale.x = std::lerp(11.0f, 13.0f, eased);
	scale.y = std::lerp(11.0f, 13.0f, eased);

	textModel_->SetScale(scale);

	textModel_->Update();
}

void TitleText::Draw() { textModel_->Draw(); }