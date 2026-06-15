#include "Goal.h"

void Goal::Initialize(EngineContext* ctx) {
	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {205.0f, 0.0f, 0.0f};

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");
	render_->SetColor({0.0f, 1.0f, 0.0f, 1.0f});
	render_->SetTransform(transform_);
}

void Goal::Update() {
	Vector3 pos = transform_.translate;

	// 当たり判定更新
	col_.max = {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f};
	col_.min = {pos.x - 0.5f, pos.y - 0.5f, pos.z - 0.5f};

	// 描画用インスタンス更新
	render_->Update(transform_);

#ifdef USE_IMGUI
	ImGui::Begin("Goal");
	ImGui::DragFloat3("Translate", &transform_.translate.x, 1.0f);
	ImGui::End();
#endif // USE_IMGUI
}

void Goal::Draw() {
	// 描画
	render_->Draw();
}