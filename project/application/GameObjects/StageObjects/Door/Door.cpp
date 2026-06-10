#include "Door.h"

void Door::Initialize(EngineContext* ctx) {
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");
}

void Door::Update(float deltaTime) {
	render_->Update();
}

void Door::Draw() {
	render_->Draw();
}
