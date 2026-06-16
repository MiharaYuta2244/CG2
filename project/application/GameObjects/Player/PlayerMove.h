#pragma once
#include "Transform.h"
#include "Vector2.h"

/// <summary>
/// プレイヤーの移動処理をまとめたクラス
/// </summary>
class PlayerMove {
public:
	// 更新処理
	void Update(Transform* transform, Vector2 dir, float deltaTime);

private:
	// 回転スピード
	float rotationSpeed_ = 4.0f;

	// 移動速度
	float moveSpeed_ = 10.0f;
};
