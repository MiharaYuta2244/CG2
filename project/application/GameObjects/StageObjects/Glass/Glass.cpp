#include "Glass.h"
#include "ColorPalette.h"
#include "Random.h"

using namespace TinyEngine;
int Glass::index = 0;

Glass::Glass() { id_ = index++; }

void Glass::Initialize(EngineContext* ctx, const Transform& transform, TinyEngine::DecalManager* decalManager) {
	transform_ = transform;
	// 生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");
	render_->SetColor({1.0f, 1.0f, 1.0f, 0.2f});

	glassesDecalManager_ = decalManager;
}

void Glass::Update() {
	// 当たり判定の更新
	UpdateCollision();

	// 更新
	render_->Update(transform_);

	// ギズモ用当たり判定更新
	UpdateAABBForGizmo();
}

void Glass::Draw() {
	// 描画
	render_->Draw();
}

void Glass::UpdateCollision() {
	Vector3 pos = transform_.translate;
	Vector3 scale = transform_.scale;
	collision_.max = {pos.x + scale.x, pos.y, pos.z + scale.z};
	collision_.min = {pos.x - scale.x, pos.y, pos.z - scale.z};
}

void Glass::AddGlassesDecal(Vector3 scale) {
	for (int i = 0; i < 50; ++i) {
		Vector3 basePos = transform_.translate;
		Vector3 pos = {
		    basePos.x + RandomUtils::RangeFloat(0.0f, 4.0f),
		    0,
		    basePos.z + RandomUtils::RangeFloat(-2.0f, 2.0f),
		};
		Vector3 rotate = {std::numbers::pi_v<float> / 2.0f, RandomUtils::RangeFloat(0.0f, std::numbers::pi_v<float>), 0.0f};
		glassesDecalManager_->AddDecal("white.png", pos, {rotate.x, rotate.y, rotate.z}, scale, ColorPalette::PastelBlue());
	}
}

void Glass::SetEnableOutline(bool isEnable) {
	if (render_) {
		render_->SetEnableOutline(isEnable);
	}
}