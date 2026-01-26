#include "Enemy.h"

void Enemy::Initialize(EngineContext* ctx) {
	// コンテキスト構造体
	ctx_ = ctx;

	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();

	// Object3dの初期化
	object3d_->Initialize(ctx_);
	object3d_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	transform_.scale = {2.0f, 2.0f, 2.0f};
	transform_.rotate = {0.0f, std::numbers::pi_v<float>, 0.0f};
	transform_.translate = {50.0f, -1.0f, 0.0f};
	size_ = {1.0f, 1.0f, 1.0f};
	collisionSize_ = {1.0f, 1.0f, 1.0f};
	isActive_ = true;

	// HP
	hp_ = kMaxHP;

	// 当たり判定の位置更新
	UpdateCollisionPos();

	// HPゲージスプライト管理クラス
	hpGauge_ = std::make_unique<EnemyHPGauge>();
	hpGauge_->Initialize(ctx_);
}

void Enemy::Update(float deltaTime) {
	// 経過時間
	deltaTime_ = deltaTime;

	// 左右移動
	HorizontalMove();

	// 反転処理
	DirectionChange();

	// 当たり判定の位置更新
	UpdateCollisionPos();

	// プレイヤーからヒップドロップを受けた時にHP減算 & 無敵フラグを立てる
	HitPlayerHipDrop();

	// 無敵時間を更新　上限に達したら無敵フラグを立てる無敵フラグを下ろす
	FrameCountIsInvincible();

	// 位置の更新
	object3d_->SetTransform(transform_);

	// 被ダメージ時のアニメーション
	UpdateDamageAnimation();

	// Object3dの更新
	object3d_->Update();

	// HPゲージスプライトサイズ更新
	hpGauge_->HPBarSpriteApply(hp_, kMaxHP);

	// HPゲージスプライト管理クラス
	hpGauge_->Update(deltaTime);
}

void Enemy::Draw() {
	// 無敵状態時に点滅
	if (invincibleFrameCount_ % 2 == 0) {
		object3d_->Draw();
	}

	// HPゲージスプライト管理クラス
	hpGauge_->Draw();
}

void Enemy::UpdateImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Enemy");

	ImGui::Text("HP : %d", hp_);
	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat2("Velocity", &velocity_.x, 0.01f);

	ImGui::End();
#endif
}

void Enemy::HorizontalMove() {
	if (direction_ == Direction::RIGHT) {
		transform_.translate.x += velocity_.x * deltaTime_;
	} else if (direction_ == Direction::LEFT) {
		transform_.translate.x -= velocity_.x * deltaTime_;
	}
}

void Enemy::DirectionChange() {
	if (transform_.translate.x >= 50.0f) {
		direction_ = Direction::LEFT;
	} else if (transform_.translate.x <= -10.0f) {
		direction_ = Direction::RIGHT;
	}
}

void Enemy::UpdateCollisionPos() {
	// 左
	collisionLeftSide_.min = {transform_.translate.x - 1.0f, transform_.translate.y - 0.5f, transform_.translate.z - 0.5f};
	collisionLeftSide_.max = {transform_.translate.x - 2.0f, transform_.translate.y + 0.5f, transform_.translate.z + 0.5f};

	// 右
	collisionRightSide_.min = {transform_.translate.x + 1.0f, transform_.translate.y - 0.5f, transform_.translate.z - 0.5f};
	collisionRightSide_.max = {transform_.translate.x + 2.0f, transform_.translate.y + 0.5f, transform_.translate.z + 0.5f};

	// 上
	aabb_.min = {transform_.translate.x - 1.5f, transform_.translate.y + 1.0f, transform_.translate.z - 0.5f};
	aabb_.max = {transform_.translate.x + 1.5f, transform_.translate.y + 2.0f, transform_.translate.z + 0.5f};

	sphere_.center = transform_.translate;
	sphere_.radius = 0.5f;
}

void Enemy::HitPlayerHipDrop() {
	if (isHitPlayerHipDrop_ && !isInvincible_) {
		// 無敵フラグを立てる
		isInvincible_ = true;

		// 被ダメージ時のアニメーション初期設定
		SettingDamageAnimation();
	}
}

void Enemy::FrameCountIsInvincible() {
	if (isInvincible_) {
		invincibleFrameCount_++;

		// 無敵時間の上限に達したら
		if (invincibleFrameCount_ >= kInvincibleFrame_) {
			isHitPlayerHipDrop_ = false; // プレイヤーからヒップドロップを受けた時に立つフラグを下ろす
			isInvincible_ = false;       // 無敵フラグを下ろす
			invincibleFrameCount_ = 0;
		}
	}
}

void Enemy::SettingDamageAnimation() {
	damageAnimation_.anim = { 
		object3d_->GetScale(), {1.0f, 1.0f, 1.0f},
         1.0f, EaseType::EASEOUTELASTIC
    };
}

void Enemy::UpdateDamageAnimation() {
	if (damageAnimation_.anim.GetIsActive()) {
		damageAnimation_.anim.Update(deltaTime_, damageAnimation_.temp);
		object3d_->SetScale(damageAnimation_.temp);
	}
}

void Enemy::SubHP(int damage) {
	// HP減算処理
	hp_ -= damage;

	hp_ = std::max(hp_, 0);
}