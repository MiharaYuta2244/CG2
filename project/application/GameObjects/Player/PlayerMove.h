#pragma once
#include "Transform.h"
#include "Vector2.h"

/// <summary>
/// プレイヤーの移動処理をまとめたクラス
/// </summary>
class PlayerMove {
public:
	// 更新処理
	void Update(Transform* transform, Vector2 inputDir, Vector2 aimDir, float deltaTime, float speedMultiplier = 1.0f, bool isHoldingEnemy = false);

	// 速度の取得
	Vector2 GetVelocity() const { return velocity_; }

	// 速度をリセットする関数
	void ResetVelocity() { velocity_ = {0.0f, 0.0f}; }

private:
	// 回転スピード
	float rotationSpeed_ = 4.0f;
	float rotateMultiplier_ = 1.0f;

	// 加速度移動用のパラメータ
	Vector2 velocity_ = {0.0f, 0.0f}; // 移動速度
	float acceleration_ = 30.0f;      // 加速度
	float friction_ = 35.0f;          // 摩擦

	// スティックの倒し具合に応じた3段階の最大速度
	float maxSpeedWalk_ = 6.0f;  // 小さく倒した時
	float maxSpeedRun_ = 8.0f;   // 中くらい倒した時
	float maxSpeedDash_ = 10.0f; // 最大まで倒した時
};
