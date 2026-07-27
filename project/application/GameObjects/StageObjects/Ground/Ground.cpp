#include "Ground.h"

void Ground::Initialize(EngineContext* ctx) {
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "plane.obj");
	render_->SetColor(color_);

	transform_.scale = {800.0f, 400.0f, 1.0f};
	transform_.rotate = {-std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f};
	transform_.translate = {transform_.scale.x / 2.0f, -0.5f, 0.0f};
}

void Ground::Update() {
	render_->SetColor(color_);
	render_->Update(transform_);
}

void Ground::Draw() { render_->Draw(); }

void Ground::SetEnableOutline(bool isEnable) {
	if (render_) {
		render_->SetEnableOutline(isEnable);
	}
}