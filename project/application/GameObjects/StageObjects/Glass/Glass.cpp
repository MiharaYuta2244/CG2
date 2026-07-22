#include "Glass.h"
#include "ColorPalette.h"

using namespace TinyEngine;
int Glass::index = 0;

Glass::Glass() { id_ = index++; }

void Glass::Initialize(EngineContext* ctx, GlassStatus glassStatus, TinyEngine::DecalManager* decalManager) {
	transform_.scale = {glassStatus.width, 50.0f, glassStatus.depth};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {glassStatus.centerX, 0.0f, glassStatus.centerZ};
	glassStatus_ = glassStatus;

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

void Glass::SetGlassStatus(GlassStatus glassStatus) {
	transform_.scale.x = glassStatus.width;
	transform_.scale.z = glassStatus.depth;
	transform_.translate.x = glassStatus.centerX;
	transform_.translate.z = glassStatus.centerZ;
	glassStatus_ = glassStatus;
}

void Glass::AddGlassesDecal(Vector3 scale) {
	Vector3 basePos = transform_.translate;
	Vector3 finalPos;
	finalPos.x = basePos.x;
	finalPos.y = 0.1f;
	finalPos.z = basePos.z;
	glassesDecalManager_->AddDecal("glasses.png", finalPos, {std::numbers::pi_v<float> / 2.0f, 0, 0}, scale, ColorPalette::PastelBlue());
}