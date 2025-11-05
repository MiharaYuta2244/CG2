#include <numbers>
#include "Player.h"
#include "GamePad.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "TextureManager.h"

void Player::Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane, DirectInput* input, GamePad* gamePad) {
	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();

	// Object3dの初期化
	object3d_->Initialize(obj3dCommon, texMane, ModelMane);

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {20.0f, 0.0f, 0.0f};
	size_ = {1.0f, 1.0f, 1.0f};
	collisionSize = {1.0f, 1.0f, 1.0f};
	isActive_ = true;

	input_ = input;

	gamePad_ = gamePad;
}

void Player::Update(float deltaTime) {
	gamePad_->Update();

	// 経過時間
	deltaTime_ = deltaTime;

	// 重力
	if (!isRotate_) {
		velocity_.y += acceleration_.y * deltaTime_;
	}

	// 横移動
	HorizontalMove();

	// ヒップドロップ
	HipDrop();

	// ヒップドロップアニメーション
	AnimationHipDrop();

	// ジャンプ
	Jump();

	// Y座標更新
	transform_.translate.y += velocity_.y * deltaTime_;

	// プレイヤーの高さが0以下にならないようにする
	if (transform_.translate.y <= groundPosY_) {
		transform_.translate.y = groundPosY_;
		velocity_.y = 0.0f;
		transform_.rotate.z = 0.0f;
		isJump_ = false;
		isRotate_ = false;
	}

	// 位置の更新
	object3d_->SetTransform(transform_);

	// Object3dの更新
	object3d_->Update();
}

void Player::Draw() { object3d_->Draw(); }

void Player::UpdateImGui() {
	ImGui::Begin("Player");

	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat2("Velocity", &velocity_.x, 0.01f);

	ImGui::End();
}

void Player::HorizontalMove() {
	bool isRightInput = input_->KeyDown(DIK_D) || gamePad_->GetState().axes.lx > 0.3f;
	bool isLeftInput = input_->KeyDown(DIK_A) || gamePad_->GetState().axes.lx < -0.3f;

	// 右移動
	if (isRightInput) {
		transform_.translate.x += velocity_.x * deltaTime_;

		// ヒップドロップアニメーション中は方向を変えられないようにする
		if (!isRotate_) {
			direction_ = Direction::RIGHT;
		}
	}

	// 左移動
	if (isLeftInput) {
		transform_.translate.x -= velocity_.x * deltaTime_;

		// ヒップドロップアニメーション中は方向を変えられないようにする
		if (!isRotate_) {
			direction_ = Direction::LEFT;
		}
	}
}

void Player::Jump() {
	bool isJumpInput = input_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;

	// ジャンプ
	if (isJumpInput && !isJump_) {
		velocity_.y = jumpPower_;
		isJump_ = true;
	}
}

void Player::HipDrop() {
	bool isHipDropInput = input_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;

	// ジャンプ中であれば
	if (isJump_ && isHipDropInput) {
		isRotate_ = true;
		velocity_.y = 0.0f;
	}
}

void Player::AnimationHipDrop() {
	// ジャンプか回転していなければ早期リターン
	if (!isJump_ || !isRotate_)
		return;

	// 回転速度
	constexpr float baseRotationSpeed = std::numbers::pi_v<float> * 3.0f;
	float rotationSpeed = (direction_ == Direction::RIGHT) ? -baseRotationSpeed : baseRotationSpeed;

	// 時間経過に応じて回転を加算
	transform_.rotate.z += rotationSpeed * deltaTime_;

	// 0〜2πの範囲に収める
	if (std::abs(transform_.rotate.z) > std::numbers::pi_v<float>) {
		transform_.rotate.z = (direction_ == Direction::RIGHT) ? -std::numbers::pi_v<float> : std::numbers::pi_v<float>;
		velocity_.y = hipDropPower_; // ヒップドロップ
	}
}
