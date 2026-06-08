#include "ExclamationMark.h"

void ExclamationMark::Initialize(EngineContext* ctx, Vector3 translate) {
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "plane.obj");
	render_->SetTexture("extension.png");

	margin_ = {1.5f, 1.0f, 1.5f};
	pos_ = {translate.x + margin_.x, translate.y + margin_.y, translate.z + margin_.z};
	transform_.scale = {1, 0, 1};
	transform_.rotate = {-std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float>, 0};
	transform_.translate = pos_;
	scaleAnim_.anim.Start(0.0f, 1.0f, 0.4f, EaseType::EASEOUTCUBIC);
}

void ExclamationMark::Update(float deltaTime, Vector3 enemyPos) {
	scaleAnim_.anim.Update(deltaTime, scaleAnim_.temp);
	transform_.scale = {1, scaleAnim_.temp, 1};
	pos_ = {enemyPos.x + margin_.x, enemyPos.y + margin_.y, enemyPos.z + margin_.z};
	transform_.translate = pos_;
	render_->SetTransform(transform_);
	render_->Update();
}

void ExclamationMark::Draw() { render_->Draw(); }
