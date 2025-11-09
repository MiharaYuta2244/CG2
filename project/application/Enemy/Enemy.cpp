#include "Enemy.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "TextureManager.h"

void Enemy::Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane) {
	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();

	// Object3dの初期化
	object3d_->Initialize(obj3dCommon, texMane, ModelMane);

	transform_.scale = {2.0f, 2.0f, 2.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {20.0f, 2.0f, 0.0f};
	size_ = {1.0f, 1.0f, 1.0f};
	collisionSize = {1.0f, 1.0f, 1.0f};
	isActive_ = true;
}

void Enemy::Update(float deltaTime) {
	// 経過時間
	deltaTime_ = deltaTime;

	// 位置の更新
	object3d_->SetTransform(transform_);

	// Object3dの更新
	object3d_->Update();
}

void Enemy::Draw() { object3d_->Draw(); }

void Enemy::UpdateImGui() {
	ImGui::Begin("Enemy");

	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat2("Velocity", &velocity_.x, 0.01f);

	ImGui::End();
}

void Enemy::HorizontalMove() {

}