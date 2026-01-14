#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class GunTurret {
public:
	void Initialize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

private:
	enum class State {
		Hidden,       // 隠れる
		Appearing,    // 出現
		Aiming,       // 狙い
		Charging,     // タメ
		Firing,       // 発射
		Disappearing, // 退場
	};

private:
	// 隠れる
	void Hide();

	// ランダムな位置に出現
	void RandomAppear();

	// ターゲットに狙いを定める
	void AimAtTarget();

	// タメ行動を行う
	void Charge();

	// 弾を発射する
	void Shot();

	// 画面外に消える
	void Disappear();

	// タイマーリセット
	void ResetTimer();

	// タメアニメーション
	void ChargeAnimation();

	// イージング移動
	void EasingMove(const Vector3 startPos, const Vector3 endPos);

private:
	// 砲台モデル
	std::unique_ptr<Object3d> gunTurretModel_;

	// 砲台の状態
	State state_ = State::Hidden; // 初期状態は隠れる

	// 経過時間計測
	float timer_ = 0.0f;

	// 出現までの時間
	const float kAppearDelay = 2.0f;

	// エイム時間
	const float kAimDuration = 1.5f;

	// タメ時間
	const float kChargeDuration = 1.0f;

	// 発射フラグ
	bool isShot_ = false;

	// 退場までの時間
	const float kDisappearDelay = 1.0f;

	// イージング移動時間
	const float kEasingDuration = 1.0f;

	// イージング終了判定
	bool isEasingComplete_ = false;
	
	// イージング開始時に現在の座標を保存するための変数
	Vector3 startPos_{};

	// 開始座標保存用のフラグ
	bool isStartPosSaved_=false;
};
