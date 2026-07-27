#include "HealArea.h"

using namespace TinyEngine;

int HealArea::index = 0;

HealArea::HealArea() { id_ = index++; }

void HealArea::Initialize(EngineContext* ctx, const Transform& transform) {
	transform_ = transform;
	transform_.scale.y = 1.0f;

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

void HealArea::SetEnableOutline(bool isEnable) {
	if (render_) {
		render_->SetEnableOutline(isEnable);
	}
}