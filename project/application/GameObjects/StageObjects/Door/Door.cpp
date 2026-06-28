#include "Door.h"

void Door::Initialize(EngineContext* ctx, WallStatus wallStatus) {
	transform_.scale = {wallStatus.width, 100.0f, wallStatus.depth};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {wallStatus.centerX, 0.0f, wallStatus.centerZ};
	wallStatus_ = wallStatus;

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");
	render_->SetTransform(transform_);
	render_->SetColor(color_);
	render_->SetEnableLighting(false);
}

void Door::Update(float deltaTime) {
	// 当たり判定更新
	Vector3 pos = transform_.translate;
	collision_.max = {pos.x + collisionSize_.x, pos.y, pos.z + collisionSize_.y};
	collision_.min = {pos.x - collisionSize_.x, pos.y, pos.z - collisionSize_.y};

	render_->Update();
}

void Door::Draw() {
	if (!isOpen_)
		return;

	render_->Draw();
}

void Door::SetWallStatus(WallStatus wallStatus) {
	transform_.scale.x = wallStatus.width;
	transform_.scale.z = wallStatus.depth;
	transform_.translate.x = wallStatus.centerX;
	transform_.translate.z = wallStatus.centerZ;
	wallStatus_ = wallStatus;
}
