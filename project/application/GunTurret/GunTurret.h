#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include "Random.h"
#include "TurretBullet.h"
#include <array>
#include <memory>

class GunTurret {
public:
	void Initialize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

	// Getter
	std::vector<std::unique_ptr<TurretBullet>>& GetBullet() { return bullets_; }

	// Setter
	void SetTargetPos(Vector3 targetPos) { targetPos_ = targetPos; }

private:
	enum class State {
		HIDDEN,       // 隠れる
		APPEARING,    // 出現
		AIMING,       // 狙い
		CHARGING,     // タメ
		SHOT,         // 発射
		WAITING,      // 待機
		DISAPPEARING, // 退場
	};

	enum class RandomPos {
		TOP,   // 上
		RIGHT, // 右
		LEFT   // 左
	};

private:
	// 隠れる
	void Hide();

	// ランダムな位置に出現
	void Appear();

	// ターゲットに狙いを定める
	void AimAtTarget();

	// タメ行動を行う
	void Charge();

	// 弾を発射する
	void Shot();

	// 待機
	void Wait();

	// 画面外に消える
	void Disappear();

	// タイマーリセット
	void ResetTimer();

	// タメアニメーション
	void ChargeAnimation();

	// イージング移動
	Vector3 EasingVector3(const Vector3 start, const Vector3 end);

	// 生成位置テーブルを更新
	void UpdateRandomGeneratePosTable();

	// 退避位置テーブルを更新
	void UpdateRandomEscapePosTable();

	// 範囲外に出たら弾の削除
	void EraseBullets();

private:
	// 砲台モデル
	std::unique_ptr<Object3d> gunTurretModel_;

	// 砲台の状態
	State state_ = State::HIDDEN; // 初期状態は隠れる

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

	// 待機時間
	const float kWaitDuration = 2.0f;

	// イージング終了判定
	bool isEasingComplete_ = false;

	// イージング開始時に現在の座標を保存するための変数
	Vector3 startPos_{};

	// 開始座標保存用のフラグ
	bool isStartPosSaved_ = false;

	// スケールアニメーション開始フラグ
	bool isScaleAnimation_ = false;

	// 初期スケール
	Vector3 startScale_{};

	// 目標スケール
	Vector3 endScale_ = {2.0f, 2.0f, 2.0f};

	// 砲台の弾
	std::vector<std::unique_ptr<TurretBullet>> bullets_;

	// エンジン機能
	EngineContext* ctx_ = nullptr;

	// 弾の向かう方向
	Vector3 bulletDirection_ = {};

	// 砲台の生成位置
	Vector3 generatePos_{};

	// 退避座標
	Vector3 escapePos_{};

	// 生成位置テーブル
	std::array<Vector3, 3> kGeneratePosTable = {
	    Vector3{RandomUtils::RangeFloat(0.0f, 40.0f),                       26.0f,  0.0f}, // TOP
	    Vector3{40.0f,	                    RandomUtils::RangeFloat(0.0f, 26.0f), 0.0f}, // RIGHT
	    Vector3{0.0f,	                     RandomUtils::RangeFloat(0.0f, 26.0f), 0.0f}, // LEFT
	};

	// 退避位置テーブル
	std::array<Vector3, 3> kEscapePosTable = {
	    Vector3{RandomUtils::RangeFloat(0.0f, 8.0f),                        32.0f, 0.0f}, // TOP
	    Vector3{32.0f,	                    RandomUtils::RangeFloat(0.0f, 8.0f), 0.0f}, // RIGHT
	    Vector3{-16.0f,                       RandomUtils::RangeFloat(0.0f, 8.0f), 0.0f}, // LEFT
	};

	// 生成&退避位置
	RandomPos currentRandomPos_ = RandomPos::TOP;

	// 発射する弾の目標地点
	Vector3 targetPos_{};
};
