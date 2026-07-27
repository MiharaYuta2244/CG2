#include "Wall.h"

int Wall::index = 0;

Wall::Wall() { id_ = index++; }

void Wall::Initialize(EngineContext* ctx, const Transform& transform) {
	transform_ = transform;
	transform_.scale.y = 50.0f;

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");
	render_->SetTransform(transform_);
	render_->SetColor(color_);
	render_->SetEnableNoise(true);
}

void Wall::Update(float deltaTime) {
	time_ += deltaTime;
	render_->SetTime(time_);

	// 当たり判定更新
	Vector3 pos = transform_.translate;
	Vector3 scale = transform_.scale;
	collision_.max = {pos.x + scale.x, pos.y, pos.z + scale.z};
	collision_.min = {pos.x - scale.x, pos.y, pos.z - scale.z};

	// 描画用インスタンス更新
	render_->Update(transform_);

	// ギズモ用当たり判定更新
	UpdateAABBForGizmo();
}

void Wall::Draw() {
	// 描画
	render_->Draw();
}

void Wall::SetEnableOutline(bool isEnable) {
	if (render_) {
		render_->SetEnableOutline(isEnable);
	}
}