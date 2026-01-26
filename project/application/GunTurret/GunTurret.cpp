#include "GunTurret.h"
#include "Easing.h"
#include "MathOperator.h"
#include <numbers>

void GunTurret::Initialize(EngineContext* ctx) {
	gunTurretModel_ = std::make_unique<Object3d>();
	gunTurretModel_->Initialize(ctx);
	gunTurretModel_->SetModel("GunTurret.obj");
	gunTurretModel_->SetTranslate({0.0f, 0.0f, 0.0f});
	gunTurretModel_->SetScale({1.0f, 1.0f, 1.0f});
	gunTurretModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});

	// エンジン機能のポインタを保存
	ctx_ = ctx;
}

void GunTurret::Update(float deltaTime) {
	timer_ += deltaTime; // 経過時間を更新

	switch (state_) {
		// 隠れる
	case State::HIDDEN:
		Hide();
		break;
		// 出現
	case State::APPEARING:
		Appear();
		break;
		// 狙い
	case State::AIMING:
		AimAtTarget();
		break;
		// タメ
	case State::CHARGING:
		Charge();
		break;
		// 発射
	case State::SHOT:
		Shot();
		break;
		// 待機
	case State::WAITING:
		Wait();
		break;
		// 退場
	case State::DISAPPEARING:
		Disappear();
		break;
	}

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update(deltaTime, bulletDirection_);
	}

	// 弾の削除
	EraseBullets();

	// Object3dの更新
	gunTurretModel_->Update();
}

void GunTurret::Draw() {
	// 砲台モデルの描画
	gunTurretModel_->Draw();

	// 弾の描画
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
}

void GunTurret::Hide() {
	if (timer_ >= kAppearDelay) {
		state_ = State::APPEARING; // 状態遷移
		ResetTimer();              // タイマーリセット

		// 事前に生成位置と退避位置を決定する
		UpdateRandomGeneratePosTable();                                          // 生成位置テーブル更新
		currentRandomPos_ = static_cast<RandomPos>(RandomUtils::RangeInt(0, 2)); // ランダムに生成位置を選択
		generatePos_ = kGeneratePosTable[static_cast<int>(currentRandomPos_)];   // ランダムに生成位置を決定
		UpdateRandomEscapePosTable();                                            // 退避位置テーブル更新
		escapePos_ = kEscapePosTable[static_cast<int>(currentRandomPos_)];       // ランダムに退避位置を決定
		gunTurretModel_->SetTranslate(escapePos_);                               // 退避位置に事前に移動
	}
}

void GunTurret::Appear() {
	// 開始位置と生成位置を保存
	if (!isStartPosSaved_) {
		startPos_ = gunTurretModel_->GetTranslate(); // 開始位置保存
		isStartPosSaved_ = true;
	}

	// 画面外からランダム座標に向かってイージング移動
	if (!isEasingComplete_) {
		gunTurretModel_->SetTranslate(EasingVector3(startPos_, generatePos_));
	}

	// イージングが終了したら
	if (isEasingComplete_) {
		ResetTimer();              // タイマーリセット
		isEasingComplete_ = false; // イージング終了フラグリセット
		isStartPosSaved_ = false;  // 開始位置保存フラグリセット
		state_ = State::AIMING;    // 状態遷移
	}
}

void GunTurret::AimAtTarget() {
	// 砲台からターゲットへのベクトル
	Vector3 direction = targetPos_ - gunTurretModel_->GetTranslate();
	direction = MathUtility::Normalize(direction);

	// 弾の発射方向を設定
	bulletDirection_ = direction;

	// Z軸の回転角度を計算
	float rotateZ = std::atan2(direction.x, direction.y);

	// 砲台のモデルの回転を設定
	gunTurretModel_->SetRotate({0.0f, 0.0f, rotateZ});

	if (timer_ >= kAimDuration) {
		state_ = State::CHARGING; // 状態遷移
		ResetTimer();
	}
}

void GunTurret::Charge() {
	// タメアニメーション
	ChargeAnimation();

	if (timer_ >= kChargeDuration) {
		gunTurretModel_->SetScale(startScale_);
		isEasingComplete_ = false; // イージング終了フラグリセット
		ResetTimer();              // タイマーリセット
		state_ = State::SHOT;      // 状態遷移
	}
}

void GunTurret::Shot() {
	// 発射処理
	bullets_.emplace_back(std::make_unique<TurretBullet>(ctx_, gunTurretModel_->GetTranslate()));

	// タイマーリセット
	ResetTimer();

	state_ = State::WAITING; // 状態遷移
}

void GunTurret::Wait() {
	if (timer_ >= kWaitDuration) {
		ResetTimer();                 // タイマーリセット
		state_ = State::DISAPPEARING; // 状態遷移
	}
}

void GunTurret::Disappear() {
	// 開始位置と退避位置を保存
	if (!isStartPosSaved_) {
		startPos_ = gunTurretModel_->GetTranslate();

		// 退避位置を設定
		UpdateRandomEscapePosTable(); // 退避位置テーブル更新
		escapePos_ = kEscapePosTable[static_cast<int>(currentRandomPos_)];

		isStartPosSaved_ = true;
	}

	// 画面外に向かってイージング移動
	if (!isEasingComplete_) {
		gunTurretModel_->SetTranslate(EasingVector3(startPos_, escapePos_));
	}

	// イージングが終了したら
	if (isEasingComplete_) {
		ResetTimer();              // タイマーリセット
		isEasingComplete_ = false; // イージング終了フラグリセット
		isStartPosSaved_ = false;  // 開始位置保存フラグリセット
		state_ = State::HIDDEN;    // 状態遷移
	}
}

void GunTurret::ResetTimer() { timer_ = 0.0f; }

void GunTurret::ChargeAnimation() {
	if (!isScaleAnimation_) {
		// 初期スケールを記録
		startScale_ = gunTurretModel_->GetScale();
		isScaleAnimation_ = true;
	}

	// イージングの結果を代入
	if (!isEasingComplete_) {
		gunTurretModel_->SetScale(EasingVector3(startScale_, endScale_));
	}
}

Vector3 GunTurret::EasingVector3(const Vector3 start, const Vector3 end) {
	// イージング移動処理
	float t = std::clamp(timer_ / kEasingDuration, 0.0f, 1.0f);

	// イージング
	float e = Easing::EaseOutCubic(t);

	// 終了判定
	if (t >= 1.0f) {
		isEasingComplete_ = true; // 終了
	}

	// 座標設定
	return start + (end - start) * e;
}

void GunTurret::UpdateRandomGeneratePosTable() {
	kGeneratePosTable = {
	    Vector3{RandomUtils::RangeFloat(0.0f, 40.0f),                       26.0f,  0.0f}, // TOP
	    Vector3{40.0f,	                    RandomUtils::RangeFloat(0.0f, 26.0f), 0.0f}, // RIGHT
	    Vector3{0.0f,	                     RandomUtils::RangeFloat(0.0f, 26.0f), 0.0f}, // LEFT
	};
}

void GunTurret::UpdateRandomEscapePosTable() {
	kEscapePosTable = {
	    Vector3{gunTurretModel_->GetTranslate().x, 36.0f,                             0.0f}, // TOP
	    Vector3{50.0f,	                         gunTurretModel_->GetTranslate().y, 0.0f}, // RIGHT
	    Vector3{-10.0f,	                        gunTurretModel_->GetTranslate().y, 0.0f}, // LEFT
	};
}

void GunTurret::EraseBullets() {
	std::erase_if(bullets_, [](const auto& b) { return b->IsOutside(); });
}
