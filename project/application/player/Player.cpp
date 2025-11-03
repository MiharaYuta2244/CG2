#include "Player.h"
#include "GamePad.h"

void Player::Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane, DirectInput* input, GamePad* gamePad) {
	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();

	// Object3dの初期化
	object3d_->Initialize(obj3dCommon, texMane, ModelMane);

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {0.0f, 0.0f, 0.0f};
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
	velocity_.y += acceleration_.y * deltaTime_;

	// 横移動
	HorizontalMove();

	// ジャンプ
	Jump();

	// Y座標更新
	transform_.translate.y += velocity_.y * deltaTime_;

	// プレイヤーの高さが0以下にならないようにする
	if (transform_.translate.y <= 0.0f) {
		transform_.translate.y = 0.0f;
		isJump_ = false;
	}

	// 位置の更新
	object3d_->SetTranslate(transform_.translate);

	// Object3dの更新
	object3d_->Update();
}

void Player::Draw() { object3d_->Draw(); }

void Player::UpdateImGui() { 
	ImGui::Begin("Player");

	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat2("Velocity", &velocity_.x, 0.01f);

	ImGui::End();
}

void Player::HorizontalMove() { 
	bool isRightInput = input_->KeyDown(DIK_D) || gamePad_->GetState().axes.lx > 0.3f;
	bool isLeftInput = input_->KeyDown(DIK_A) || gamePad_->GetState().axes.lx < -0.3f;

	// 右移動
	if (isRightInput) {
		transform_.translate.x += velocity_.x * deltaTime_;
	}

	// 左移動
	if (isLeftInput) {
		transform_.translate.x -= velocity_.x * deltaTime_;
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
