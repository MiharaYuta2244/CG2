#include "HealArea.h"

using namespace TinyEngine;

int HealArea::index = 0;

HealArea::HealArea() { id_ = index++; }

void HealArea::Initialize(EngineContext* ctx, HealAreaStatus healAreaStatus) {
	transform_.scale = {healAreaStatus.width, 0.5f, healAreaStatus.depth};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {healAreaStatus.centerX, 0.0f, healAreaStatus.centerZ};
	healAreaStatus_ = healAreaStatus;

	// 生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");
	render_->SetColor(color_);
}

void HealArea::Update(float deltaTime) {
	// 当たり判定更新
	Vector3 pos = transform_.translate;
	Vector3 scale = transform_.scale;
	collision_.max = {pos.x + scale.x, pos.y, pos.z + scale.z};
	collision_.min = {pos.x - scale.x, pos.y, pos.z - scale.z};

	// 更新
	render_->Update(transform_);
}

void HealArea::Draw() {
	if (!isActive_)
		return;

	// 描画
	render_->Draw();
}

void HealArea::SetHealAreaStatus(HealAreaStatus HealAreaStatus) {
	transform_.scale.x = HealAreaStatus.width;
	transform_.scale.z = HealAreaStatus.depth;
	transform_.translate.x = HealAreaStatus.centerX;
	transform_.translate.z = HealAreaStatus.centerZ;
	healAreaStatus_ = HealAreaStatus;
}
