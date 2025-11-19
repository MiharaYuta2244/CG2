#include "Player.h"
#include "GamePad.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "MathUtility.h"
#include "MathOperator.h"
#include <numbers>

void Player::Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane, DirectInput* input, GamePad* gamePad, SpriteCommon* spriteCommon) {
	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();

	object3dCommon_ = obj3dCommon;

	// Object3dの初期化
	object3d_->Initialize(obj3dCommon, texMane, ModelMane);
	object3d_->SetColor({1.0f, 0.0f, 0.0f, 1.0f});

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {20.0f, 0.0f, 0.0f};
	size_ = {1.0f, 1.0f, 1.0f};
	collisionSize = {1.0f, 1.0f, 1.0f};
	isActive_ = true;

	// 当たり判定位置更新
	UpdateCollisionPos();

	// 入力
	input_ = input;
	gamePad_ = gamePad;

	// HP
	hp_ = 5;

	// HPゲージスプライトサイズ設定
	spriteHPGauge_.resize(hp_);

	// HPゲージスプライト
	for (int i = 0; i < spriteHPGauge_.size(); ++i) {
		spriteHPGauge_[i] = std::make_unique<Sprite>();
		spriteHPGauge_[i]->Initialize(spriteCommon, texMane, "resources/white.png");
		spriteHPGauge_[i]->SetPosition({70.0f * i + 32.0f, 32.0f});
		spriteHPGauge_[i]->SetSize({64.0f, 64.0f});
		spriteHPGauge_[i]->SetColor({0.0f, 1.0f, 0.0f, 1.0f});
	}

	// HPゲージ背景スプライト
	spriteHPGaugeBG_->Initialize(spriteCommon, texMane, "resources/white.png");
	spriteHPGaugeBG_->SetPosition({22.0f, 22.0f});
	spriteHPGaugeBG_->SetSize({364.0f, 84.0f});
	spriteHPGaugeBG_->SetColor({0.0f, 0.0f, 0.0f, 1.0f});
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

	// ヒップドロップが当たった後の処理
	AfterHipDrop();

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
	}

	// 敵との当たり判定が取れた時にHP減算 & 無敵フラグを立てる
	HitEnemy();

	// 無敵時間を更新　上限に達したら無敵フラグを立てる無敵フラグを下ろす
	FrameCountIsInvincible();

	// 当たり判定位置更新
	UpdateCollisionPos();

	// HPゲージスプライト
	for (auto& hpGauge : spriteHPGauge_) {
		hpGauge->Update();
	}

	// スプライトに変換した座標をセットする
	spriteHPGaugeBG_->SetPosition(ScreenToWorldPoint(transform_.translate, spriteMargin_));

	// HPゲージ背景スプライト
	spriteHPGaugeBG_->Update();

	// 位置の更新
	object3d_->SetTransform(transform_);

	// Object3dの更新
	object3d_->Update();
}

void Player::Draw() {
	// 無敵状態時は点滅
	if (invincibleFrameCount_ % 2 == 0) {
		object3d_->Draw();
	}

	// HPゲージ背景スプライト
	spriteHPGaugeBG_->Draw();

	// HPゲージスプライト
	for (auto& hpGauge : spriteHPGauge_) {
		hpGauge->Draw();
	}
}

void Player::UpdateImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Player");

	ImGui::Text("HP : %d", hp_);
	ImGui::Text("InvincibleFrameCount : %d", invincibleFrameCount_);
	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat2("Velocity", &velocity_.x, 0.01f);
	ImGui::DragFloat3("direction", &object3d_->GetDirectionalLight().direction.x, 0.01f);
	ImGui::DragFloat("intensity", &object3d_->GetDirectionalLight().intensity, 0.01f);
	ImGui::DragFloat("shininess", &object3d_->GetMaterial().shininess, 0.01f);

	ImGui::End();
#endif
}

void Player::HorizontalMove() {
	if (isRotate_)
		return;

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
	// どっちがいいかわからない!!!!!!!!!!!!
	bool isHipDropInput = input_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a; // スペース
	// bool isHipDropInput = input_->KeyDown(DIK_S) || gamePad_->GetState().axes.ly > 0.3f; // 下

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
	constexpr float baseRotationSpeed = std::numbers::pi_v<float> * 6.0f;
	float rotationSpeed = (direction_ == Direction::RIGHT) ? -baseRotationSpeed : baseRotationSpeed;

	// 時間経過に応じて回転を加算
	transform_.rotate.z += rotationSpeed * deltaTime_;

	// 0〜2πの範囲に収める
	if (std::abs(transform_.rotate.z) > std::numbers::pi_v<float> * 2) {
		transform_.rotate.z = (direction_ == Direction::RIGHT) ? -std::numbers::pi_v<float> * 2 : std::numbers::pi_v<float> * 2;
		velocity_.y = hipDropPower_; // ヒップドロップ
		isRotate_ = false;
	}
}

void Player::UpdateCollisionPos() {
	// 当たり判定の更新
	aabb_.min = {transform_.translate.x - 0.5f, transform_.translate.y - 0.5f, transform_.translate.z - 0.5f};
	aabb_.max = {transform_.translate.x + 0.5f, transform_.translate.y + 0.5f, transform_.translate.z + 0.5f};
	sphere_.center = transform_.translate;
	sphere_.radius = 0.5f;
}

void Player::HitEnemy() {
	// HPが0以下の時は早期リターン
	if (hp_ <= 0)
		return;

	if (isHitEnemy_ && !isInvincible_) {
		// HP減算
		SubHP();

		// HPゲージスプライトのサイズ変更
		spriteHPGauge_.resize(hp_);

		// 無敵フラグを立てる
		isInvincible_ = true;
	}
}

void Player::FrameCountIsInvincible() {
	if (isInvincible_) {
		invincibleFrameCount_++;

		// 無敵時間の上限に達したら
		if (invincibleFrameCount_ >= kInvincibleFrame_) {
			isHitEnemy_ = false;   // 敵との当たり判定フラグを下ろす
			isInvincible_ = false; // 無敵フラグを下ろす
			invincibleFrameCount_ = 0;
		}
	}
}

void Player::SubHP() {
	// HP減算
	hp_--;
}

void Player::AfterHipDrop() {
	if (isHitEnemyHipDrop_) {
		// 打ち上げ
		velocity_.y = 20.0f;

		// 回転をリセット
		transform_.rotate.z = 0.0f;

		// 敵にヒップドロップを当てた時のフラグを下ろす
		isHitEnemyHipDrop_ = false;
	}
}

Vector2 Player::ScreenToWorldPoint(Vector3 worldPosition, Vector2 margin) { 
	// ビューポートマトリックス
	Matrix4x4 viewportMatrix = MathUtility::MakeViewPortMatrix(0, 0, 1280.0f, 720.0f, 0.0f, 1.0f);

	// カメラのViewProjectionとビューポートを合成してワールド→スクリーン変換行列を作る
	Matrix4x4 matVPV = MathUtility::Multiply(object3dCommon_->GetDefaultCamera()->GetViewProjectionMatrix(), viewportMatrix);

	// プレイヤーのワールド座標をスクリーン座標に変換
	Vector3 worldPos = {transform_.translate.x + spriteMargin_.x, transform_.translate.y + spriteMargin_.y, transform_.translate.z};
	Vector3 screenPos = MathUtility::Transform(worldPos, matVPV);

	return {screenPos.x, screenPos.y};
}
