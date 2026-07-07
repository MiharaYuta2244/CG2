#include "Cage.h"

void Cage::Initialize(EngineContext* ctx, CageStatus cageStatus) {
	transform_.scale = {cageStatus.width, 50.0f, cageStatus.depth};
	transform_.rotate = {0.0f, cageStatus.rotateY, 0.0f};
	transform_.translate = {cageStatus.centerX, 0.0f, cageStatus.centerZ};
	cageStatus_ = cageStatus;

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cage.obj");
	render_->SetTransform(transform_);
	render_->SetColor(color_);
	render_->SetEnableLighting(false);
}

void Cage::Update(float deltaTime) {
	time_ += deltaTime;
	render_->SetTime(time_);

	// 当たり判定更新
	Vector3 pos = transform_.translate;
	Vector3 scale = transform_.scale;
	collision_.max = {pos.x + scale.x, pos.y, pos.z + scale.z};
	collision_.min = {pos.x - scale.x, pos.y, pos.z - scale.z};

	// 描画用インスタンス更新
	render_->Update(transform_);
}

void Cage::Draw() {
	// 描画
	render_->Draw();
}

void Cage::SetCageStatus(CageStatus cageStatus) {
	transform_.scale.x = cageStatus.width;
	transform_.scale.z = cageStatus.depth;
	transform_.translate.x = cageStatus.centerX;
	transform_.translate.z = cageStatus.centerZ;
	transform_.rotate.y = cageStatus.rotateY;
	cageStatus_ = cageStatus;
}
