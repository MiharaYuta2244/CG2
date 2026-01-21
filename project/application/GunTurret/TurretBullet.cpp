#include "TurretBullet.h"
#include "MathOperator.h"

TurretBullet::TurretBullet(EngineContext* ctx, Vector3 pos) {
	Initialize(ctx, pos);
}

void TurretBullet::Initialize(EngineContext* ctx, Vector3 pos) {
	bulletModel_ = std::make_unique<Object3d>();
	bulletModel_->Initialize(ctx);
	bulletModel_->SetModel("TurretBullet.obj");
	bulletModel_->SetTranslate(pos);
	bulletModel_->SetScale({1.0f, 1.0f, 1.0f});
	bulletModel_->SetRotate({0.0f, 0.0f, 0.0f});
}

void TurretBullet::Update(float deltaTime, Vector3 direction) {
	// 移動処理
	Move(deltaTime, direction);

	// 当たり判定の更新
	aabb_.min = {bulletModel_->GetTranslate().x - 0.5f, bulletModel_->GetTranslate().y - 0.5f, bulletModel_->GetTranslate().z - 0.5f};
	aabb_.max = {bulletModel_->GetTranslate().x + 0.5f, bulletModel_->GetTranslate().y + 0.5f, bulletModel_->GetTranslate().z + 0.5f};

	// Object3dの更新
	bulletModel_->Update();
}

void TurretBullet::Draw() { bulletModel_->Draw(); }

bool TurretBullet::IsOutside() { 
	Vector3 pos = bulletModel_->GetTranslate(); // 弾の現在位置を取得

	return pos.x < -10.0f || pos.x > 50.0f || pos.y < -10.0f || pos.y > 50.0f;
}

void TurretBullet::Move(float deltaTime, Vector3 direction) { bulletModel_->SetTranslate(bulletModel_->GetTranslate() += direction * velocity_ * deltaTime); }
