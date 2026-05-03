#include "Ground.h"

void Ground::Initialize(EngineContext* ctx) {
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "plane.obj");
	render_->SetColor(color_);

	transform_.scale = {200.0f, 200.0f, 1.0f};
	transform_.rotate = {-std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f};
	transform_.translate = {0.0f, -0.5f, 0.0f};
}

void Ground::Update() {
#ifdef USE_IMGUI
	ImGui::Begin("Ground");
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
	ImGui::ColorEdit4("Color", &color_.x);
	ImGui::End();
#endif

	render_->SetColor(color_);
	render_->Update(transform_);
}

void Ground::Draw() { render_->Draw(); }
