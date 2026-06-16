#include "PlayerMove.h"
#include <complex>
#include <numbers>

void PlayerMove::Update(Transform* transform, Vector2 inputDir, float deltaTime, float speedMultiplier) {
	Vector3& rotate = transform->rotate;

	// 入力ベクトルの長さを取得
	float inputLength = std::sqrtf(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
	bool hasInput = (inputLength > 0.1f); // デッドゾーン

	// 3段階の最大速度を決定
	float currentMaxSpeed = 0.0f;
	if (inputLength > 0.8f) {
		currentMaxSpeed = maxSpeedDash_;
	} else if (inputLength > 0.4f) {
		currentMaxSpeed = maxSpeedRun_;
	} else if (inputLength > 0.1f) {
		currentMaxSpeed = maxSpeedWalk_;
	}

	// 最大速度に倍率をかける(掴み中に減速がかかるように)
	currentMaxSpeed *= speedMultiplier;

	if (hasInput) {
		// 向き用の正規化ベクトル
		Vector2 dir = {inputDir.x / inputLength, inputDir.y / inputLength};

		// 加速度を適用
		velocity_.x += dir.x * acceleration_ * deltaTime;
		velocity_.y += dir.y * acceleration_ * deltaTime;

		// 現在の速度を計算
		float currentSpeed = std::sqrtf(velocity_.x * velocity_.x + velocity_.y * velocity_.y);

		// 決定した最大速度で制限
		if (currentSpeed > currentMaxSpeed) {
			velocity_.x = (velocity_.x / currentSpeed) * currentMaxSpeed;
			velocity_.y = (velocity_.y / currentSpeed) * currentMaxSpeed;
		}

		// 回転処理
		float targetAngle = std::atan2(dir.x, dir.y);
		float diff = targetAngle - rotate.y;

		const float kPi = std::numbers::pi_v<float>;
		while (diff < -kPi)
			diff += 2.0f * kPi;
		while (diff > kPi)
			diff -= 2.0f * kPi;

		rotate.y += diff * rotationSpeed_ * deltaTime;

	} else {
		// 入力がない場合は摩擦で減速させる
		float currentSpeed = std::sqrtf(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
		if (currentSpeed > 0.0f) {
			float drop = friction_ * deltaTime;
			// 0未満にならないように減速割合を計算
			float multiplier = std::max(0.0f, currentSpeed - drop) / currentSpeed;
			velocity_.x *= multiplier;
			velocity_.y *= multiplier;
		}
	}

	// 位置更新
	transform->translate.x += velocity_.x * deltaTime;
	transform->translate.z += velocity_.y * deltaTime;
}