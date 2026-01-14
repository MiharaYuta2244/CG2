#include "GunTurret.h"
#include <numbers>

void GunTurret::Initialize(EngineContext* ctx) {
	gunTurretModel_ = std::make_unique<Object3d>();
	gunTurretModel_->Initialize(ctx);
	gunTurretModel_->SetModel("GunTurret.obj");
	gunTurretModel_->SetTranslate({0.0f, 0.0f, 0.0f});
	gunTurretModel_->SetScale({1.0f, 1.0f, 1.0f});
	gunTurretModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void GunTurret::Update(float deltaTime) { (void)deltaTime; }

void GunTurret::Draw() { gunTurretModel_->Draw(); }