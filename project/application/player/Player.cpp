#include "Player.h"
#include "GamePad.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "TextureManager.h"
#include <numbers>

void Player::Initialize(EngineContext* ctx, DirectInput* input, GamePad* gamePad) {
	// コンテキスト構造体
	ctx_ = ctx;

	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();

	// Object3dの初期化
	object3d_->Initialize(ctx_);

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, std::numbers::pi_v<float> / 2, 0.0f};
	transform_.translate = {20.0f, 0.0f, 0.0f};
	size_ = {1.0f, 1.0f, 1.0f};
	collisionSize_ = {1.0f, 1.0f, 1.0f};
	isActive_ = true;

	// 当たり判定位置更新
	UpdateCollisionPos();

	// 入力
	input_ = input;
	gamePad_ = gamePad;

	// HP
	hp_ = kMaxHP;

	// レベルアップスプライト
	levelUpSprite_ = std::make_unique<Sprite>();
	levelUpSprite_->Initialize(ctx, "resources/LEVEL_UP.png");
	levelUpSprite_->SetPosition({640.0f, 360.0f});
	levelUpSprite_->SetSize({64.0f, 14.0f});
	levelUpSprite_->SetEnableShine(true);
	levelUpSprite_->SetShineParams({0.0f, 0.5f, 0.5f, 1.0f});

	// 十字エフェクトスプライト
	crossSprite_ = std::make_unique<Sprite>();
	crossSprite_->Initialize(ctx, "resources/Cross.png");
	crossSprite_->SetPosition({0.0f, 0.0f});
	crossSprite_->SetSize({1920.0f, 1920.0f});
	crossSprite_->SetAnchorPoint({0.5f, 0.5f});
	crossSprite_->SetColor({0.6f, 0.6f, 0.6f, 1.0f});
	crossSprite_->SetEnableShine(true);
	crossSprite_->SetShineParams({0.0f, 0.8f, 1.0f, 1.0f});
	crossSprite_->SetShineColor({1.0f, 1.0f, 1.0f, 1.0f});

	// 十字エフェクトアニメーション
	crossRotateAnimation_.anim = {0.0f, std::numbers::pi_v<float> * 2, 0.5f, EaseType::EASEOUTCUBIC};

	// HPゲージ
	hpGauge_ = std::make_unique<PlayerHPGauge>();
	hpGauge_->Initialize(ctx);
}

void Player::Update(float deltaTime) {
	// 経過時間
	deltaTime_ = deltaTime;

	// 重力
	if (!isRotate_) {
		velocity_.y += acceleration_.y * deltaTime_;
	}

	// 横移動
	HorizontalMove();

	if (transform_.translate.x >= rightMoveLimit_) {
		transform_.translate.x = rightMoveLimit_;
	} else if (transform_.translate.x <= leftMoveLimit_) {
		transform_.translate.x = leftMoveLimit_;
	}

	// ヒップドロップ
	HipDrop();

	// ヒップドロップアニメーション
	AnimationHipDrop();

	// ヒップドロップが当たった後の処理
	AfterHipDrop();

	// ジャンプ
	Jump();

	// ジャンプアニメーション
	AnimationJump();

	// フルーツ取得アニメーション
	AnimationFruitGet();

	// Y座標更新
	transform_.translate.y += velocity_.y * deltaTime_;

	// プレイヤーの高さが0以下にならないようにする
	if (transform_.translate.y <= groundPosY_) {
		transform_.translate.y = groundPosY_;
		velocity_.y = 0.0f;
		transform_.rotate.z = 0.0f;
		isJump_ = false;
		isHipDropDamage_ = false;
	}

	// 敵との当たり判定が取れた時にHP減算 & 無敵フラグを立てる
	HitEnemy();

	// 無敵時間を更新　上限に達したら無敵フラグを立てる無敵フラグを下ろす
	FrameCountIsInvincible();

	// 当たり判定位置更新
	UpdateCollisionPos();

	// HPゲージスプライト
	hpGauge_->SetPosition(ScreenToWorldPoint({transform_.translate.x, transform_.translate.y, transform_.translate.z}, spriteHPGaugeMargin_));
	hpGauge_->HPBarSpriteApply(hp_, kMaxHP);
	hpGauge_->Update(deltaTime);

	// レベルアップスプライト
	if (isLevelUp_) {
		bool playing = levelUpAnimation_.anim.Update(deltaTime, levelUpAnimation_.temp);
		levelUpSprite_->SetSize(levelUpAnimation_.temp);

		if (!playing) {
			isLevelUp_ = false; // レベルアップフラグをおろす
			levelUpSprite_->SetSize({64.0f, 14.0f});
		}

		levelUpSprite_->SetPosition(ScreenToWorldPoint({transform_.translate.x, transform_.translate.y, transform_.translate.z}, spriteLevelUpMargin_));
		levelUpSprite_->Update();
	}

	// 十字エフェクト
	bool playing = crossRotateAnimation_.anim.Update(deltaTime, crossRotateAnimation_.temp);

	if (!playing) {
		crossRotateAnimation_.anim = {0.0f, std::numbers::pi_v<float> * 2, 0.5f, EaseType::EASEOUTCUBIC};
	}

	crossSprite_->SetRotation(crossRotateAnimation_.temp);
	crossSprite_->SetPosition(ScreenToWorldPoint({transform_.translate.x, transform_.translate.y, transform_.translate.z}, {0.0f, 0.0f}));
	crossSprite_->Update();

	// 位置の更新
	object3d_->SetTransform(transform_);

	// Object3dの更新
	object3d_->Update();

	// ImGui
	UpdateImGui();
}

void Player::Draw() {
	// 無敵状態時は点滅
	if (invincibleFrameCount_ % 2 == 0) {
		object3d_->Draw();
	}

	// HPゲージスプライト
	hpGauge_->Draw();

	// 十字エフェクト
	// crossSprite_->Draw();

	// レベルアップスプライト
	if (isLevelUp_) {
		levelUpSprite_->Draw();
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

		// モデルの向きを変更
		transform_.rotate.y = std::numbers::pi_v<float> / 2;

		// ヒップドロップアニメーション中は方向を変えられないようにする
		if (!isRotate_) {
			direction_ = Direction::RIGHT;
		}
	}

	// 左移動
	if (isLeftInput) {
		transform_.translate.x -= velocity_.x * deltaTime_;

		// モデルの向きを変更
		transform_.rotate.y = -std::numbers::pi_v<float> / 2;

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

		// ジャンプ時のスケールに変更していく
		jumpScaleAnim.start = {transform_.translate};
		jumpScaleAnim.end = {1.0f, 1.5f, 1.0f};
		jumpScaleAnim.temp = {transform_.translate};
		jumpScaleAnim.anim = {transform_.scale, jumpScaleAnim.end, 0.5f, EaseType::EASEOUTCUBIC};
	}
}

void Player::HipDrop() {
	bool isHipDropInput = input_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a; // スペース

	// ジャンプ中であれば
	if (isJump_ && isHipDropInput) {
		isRotate_ = true;
		velocity_.y = 0.0f;
		isHipDropDamage_ = true;
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

		// 無敵フラグを立てる
		isInvincible_ = true;

		// 当たり判定用ヒップドロップ判定フラグを下ろす
		isHipDropDamage_ = false;
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

void Player::IncrementHipDropPowerLevel() {
	hipDropPowerLevel_ += 1; // 攻撃力を増やす
	isLevelUp_ = true;       // レベルアップフラグを立てる

	// レベルアップアニメーション初期設定
	levelUpAnimation_.anim = {
	    levelUpSprite_->GetSize(), {128.0f, 28.0f},
         1.0f, EaseType::EASEOUTBOUNCE
    };
}

void Player::SetFruitGetAnimation() {
	fruitGetAnim_.anim = {
	    transform_.scale, {1.5f, 1.5f, 1.5f},
         0.3f, EaseType::EASEOUTCUBIC
    };

	if (afterFruitGetAnim_.anim.GetIsActive()) {
		afterFruitGetAnim_.anim.Reset();
	}
}

Vector2 Player::ScreenToWorldPoint(Vector3 worldPosition, Vector2 margin) {
	// ビューポートマトリックス
	Matrix4x4 viewportMatrix = MathUtility::MakeViewPortMatrix(0, 0, 1280.0f, 720.0f, 0.0f, 1.0f);

	// カメラのViewProjectionとビューポートを合成してワールド→スクリーン変換行列を作る
	Matrix4x4 matVPV = MathUtility::Multiply(ctx_->object3dCommon->GetDefaultCamera()->GetViewProjectionMatrix(), viewportMatrix);

	// プレイヤーのワールド座標をスクリーン座標に変換
	Vector3 worldPos = {worldPosition.x + margin.x, worldPosition.y + margin.y, worldPosition.z};
	Vector3 screenPos = MathUtility::Transform(worldPos, matVPV);

	return {screenPos.x, screenPos.y};
}

void Player::AnimationJump() {
	// ジャンプ中アニメーション
	if (jumpScaleAnim.anim.GetIsActive()) {
		bool playing = jumpScaleAnim.anim.Update(deltaTime_, jumpScaleAnim.temp);
		transform_.scale = jumpScaleAnim.temp;

		// 終了したら次のアニメーションへ
		if (!playing) {
			afterJumpScaleAnim.anim = {
			    transform_.scale, {1.0f, 1.0f, 1.0f},
                 0.5f, EaseType::EASEOUTCUBIC
            };
		}
		return;
	}

	// ジャンプ後アニメーション
	if (afterJumpScaleAnim.anim.GetIsActive()) {
		afterJumpScaleAnim.anim.Update(deltaTime_, afterJumpScaleAnim.temp);
		transform_.scale = afterJumpScaleAnim.temp;
	}
}

void Player::AnimationFruitGet() {
	// フルーツ取得時のスケールアニメーション
	if (isGetFruit_ && fruitGetAnim_.anim.GetIsActive()) {
		bool playing = fruitGetAnim_.anim.Update(deltaTime_, fruitGetAnim_.temp);
		transform_.scale = fruitGetAnim_.temp;

		if (!playing && !afterFruitGetAnim_.anim.GetIsActive()) {
			isGetFruit_ = false; // 取得フラグを下す

			// 取得後アニメーション初期化
			afterFruitGetAnim_.anim = {
			    transform_.scale, {1.0f, 1.0f, 1.0f},
                 0.3f, EaseType::EASEOUTCUBIC
            };
		}
		return;
	}

	// 取得後アニメーション
	if (afterFruitGetAnim_.anim.GetIsActive()) {
		afterFruitGetAnim_.anim.Update(deltaTime_, afterFruitGetAnim_.temp);
		transform_.scale = afterFruitGetAnim_.temp;
	}
}
