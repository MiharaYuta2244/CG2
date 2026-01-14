#include "GunTurret.h"
#include "Easing.h"
#include "MathOperator.h"
#include "Random.h"
#include <numbers>

void GunTurret::Initialize(EngineContext* ctx) {
	gunTurretModel_ = std::make_unique<Object3d>();
	gunTurretModel_->Initialize(ctx);
	gunTurretModel_->SetModel("Hiyoko.obj");
	gunTurretModel_->SetTranslate({0.0f, 0.0f, 0.0f});
	gunTurretModel_->SetScale({1.0f, 1.0f, 1.0f});
	gunTurretModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void GunTurret::Update(float deltaTime) {
	timer_ += deltaTime; // 経過時間を更新

	switch (state_) {
		// 隠れる
	case State::Hidden:
		Hide();
		break;
		// 出現
	case State::Appearing:
		RandomAppear();
		break;
		// 狙い
	case State::Aiming:
		AimAtTarget();
		break;
		// タメ
	case State::Charging:
		Charge();
		break;
		// 発射
	case State::Firing:
		Shot();
		break;
		// 退場
	case State::Disappearing:
		Disappear();
		break;
	}

	// Object3dの更新
	gunTurretModel_->Update();
}

void GunTurret::Draw() { gunTurretModel_->Draw(); }

void GunTurret::Hide() {
	if (timer_ >= kAppearDelay) {
		state_ = State::Appearing; // 状態遷移
		ResetTimer();              // タイマーリセット
	}
}

void GunTurret::RandomAppear() {
	Vector3 randomTargetPos = {RandomUtils::RangeFloat(0.0f, 1.0f), RandomUtils::RangeFloat(0.0f, 1.0f), RandomUtils::RangeFloat(0.0f, 1.0f)};

	// 開始位置を保存
	if (!isStartPosSaved_) {
		startPos_ = gunTurretModel_->GetTranslate();
		isStartPosSaved_ = true;
	}

	// 画面外からランダム座標に向かってイージング移動
	EasingMove(startPos_, randomTargetPos);

	// イージングが終了したら
	if (isEasingComplete_) {
		ResetTimer();              // タイマーリセット
		isEasingComplete_ = false; // イージング終了フラグリセット
		isStartPosSaved_ = false;  // 開始位置保存フラグリセット
		state_ = State::Aiming;    // 状態遷移
	}
}

void GunTurret::AimAtTarget() {
	state_ = State::Charging; // 状態遷移
}

void GunTurret::Charge() {
	// タメアニメーション
	ChargeAnimation();

	if (timer_ >= kChargeDuration) {
		ResetTimer();           // タイマーリセット
		state_ = State::Firing; // 状態遷移
	}
}

void GunTurret::Shot() {
	// 発射処理

	state_ = State::Disappearing; // 状態遷移
}

void GunTurret::Disappear() {
	Vector3 randomTargetPos = {RandomUtils::RangeFloat(0.0f, 1.0f), RandomUtils::RangeFloat(0.0f, 1.0f), RandomUtils::RangeFloat(0.0f, 1.0f)};

	// 開始位置を保存
	if (!isStartPosSaved_) {
		startPos_ = gunTurretModel_->GetTranslate();
		isStartPosSaved_ = true;
	}

	// 画面外に向かってイージング移動
	EasingMove(startPos_, randomTargetPos);

	// イージングが終了したら
	if (isEasingComplete_) {
		ResetTimer();              // タイマーリセット
		isEasingComplete_ = false; // イージング終了フラグリセット
		isStartPosSaved_ = false;  // 開始位置保存フラグリセット
		state_ = State::Hidden;    // 状態遷移
	}
}

void GunTurret::ResetTimer() { timer_ = 0.0f; }

void GunTurret::ChargeAnimation() {}

void GunTurret::EasingMove(const Vector3 startPos, const Vector3 endPos) {
	if (!isEasingComplete_)
		return;

	// イージング移動処理
	float t = std::clamp(timer_ / kEasingDuration, 0.0f, 1.0f);

	// イージング
	float e = Easing::easeOutCubic(t);

	// 座標設定
	gunTurretModel_->SetTranslate(startPos + (endPos - startPos) * e);

	// 終了判定
	if (t >= 1.0f) {
		isEasingComplete_ = true; // 終了
	}
}
