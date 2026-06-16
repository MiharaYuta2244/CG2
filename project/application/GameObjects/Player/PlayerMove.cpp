#include "PlayerMove.h"
#include <complex>
#include <numbers>

void PlayerMove::Update(Transform* transform, Vector2 dir, float deltaTime) {
	Vector3& rotate = transform->rotate;

	// 正規化
	float length = std::sqrtf(dir.x * dir.x + dir.y * dir.y);
	bool isMoving = (length > 0.0f);

	if (isMoving) {
		dir.x /= length;
		dir.y /= length;

		// 目標角度（Yaw）
		float targetAngle = std::atan2(dir.x, dir.y);

		// 現在角度との差分
		float diff = targetAngle - rotate.y;

		const float kPi = std::numbers::pi_v<float>;
		while (diff < -kPi)
			diff += 2.0f * kPi;
		while (diff > kPi)
			diff -= 2.0f * kPi;

		// 回転補間
		rotate.y += diff * rotationSpeed_ * deltaTime;

		// 位置更新
		transform->translate.x += dir.x * moveSpeed_ * deltaTime;
		transform->translate.z += dir.y * moveSpeed_ * deltaTime;
	}
}