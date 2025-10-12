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

void Player::Update() {
	gamePad_->Update();

	// 重力
	velocity_.y += acceleration_.y;
	transform_.translate.y += velocity_.y;

	// 横移動
	HorizontalMove();

	// ジャンプ
	Jump();

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

void Player::HorizontalMove() { 
	bool isRightInput = input_->KeyDown(DIK_D) || gamePad_->GetState().axes.lx > 0.3f;
	bool isLeftInput = input_->KeyDown(DIK_A) || gamePad_->GetState().axes.lx < -0.3f;

	// 右移動
	if (isRightInput) {
		transform_.translate.x += 0.1f;
	}

	// 左移動
	if (isLeftInput) {
		transform_.translate.x -= 0.1f;
	}
}

void Player::Jump() { 
	bool isJumpInput = input_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;

	// ジャンプ
	if (isJumpInput && !isJump_) {
		velocity_.y = 1.0f;
		isJump_ = true;
	}
}
