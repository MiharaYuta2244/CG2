#include "PlayerMove.h"
#include "ImGuiManager.h"
#include <complex>
#include <numbers>

void PlayerMove::Update(Transform* transform, Vector2 inputDir, Vector2 aimDir, float deltaTime, float speedMultiplier, bool isHoldingEnemy) {
	Vector3& rotate = transform->rotate;

	// 入力ベクトルの長さを取得
	float inputLength = std::sqrtf(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
	bool hasInput = (inputLength > 0.1f); // デッドゾーン

	// Rスティックベクトルの長さを取得
	float aimLength = std::sqrtf(aimDir.x * aimDir.x + aimDir.y * aimDir.y);
	bool hasAimInput = (aimLength > 0.1f);

	// 移動速度の計算
	float currentMaxSpeed = 0.0f;
	if (inputLength > 0.8f) {
		currentMaxSpeed = maxSpeedDash_;
	} else if (inputLength > 0.4f) {
		currentMaxSpeed = maxSpeedRun_;
	} else if (inputLength > 0.1f) {
		currentMaxSpeed = maxSpeedWalk_;
	}
	
	currentMaxSpeed *= speedMultiplier;

	if (hasInput) {
		Vector2 dir = {inputDir.x / inputLength, inputDir.y / inputLength};
		velocity_.x += dir.x * acceleration_ * deltaTime;
		velocity_.y += dir.y * acceleration_ * deltaTime;

		float currentSpeed = std::sqrtf(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
		if (currentSpeed > currentMaxSpeed) {
			velocity_.x = (velocity_.x / currentSpeed) * currentMaxSpeed;
			velocity_.y = (velocity_.y / currentSpeed) * currentMaxSpeed;
		}
	} else {
		// 入力がない場合は摩擦で減速させる
		float currentSpeed = std::sqrtf(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
		if (currentSpeed > 0.0f) {
			float drop = friction_ * deltaTime;
			float multiplier = std::max(0.0f, currentSpeed - drop) / currentSpeed;
			velocity_.x *= multiplier;
			velocity_.y *= multiplier;
		}
	}

	float rotSpeed = rotationSpeed_;
	if (isHoldingEnemy) {
		rotSpeed *= rotateMultiplier_;
	}

	// 回転処理
	if (hasAimInput) {
		Vector2 aimNorm = {aimDir.x / aimLength, aimDir.y / aimLength};
		float targetAngle = std::atan2(aimNorm.x, aimNorm.y);
		float diff = targetAngle - rotate.y;

		const float kPi = std::numbers::pi_v<float>;
		while (diff < -kPi)
			diff += 2.0f * kPi;
		while (diff > kPi)
			diff -= 2.0f * kPi;

		rotate.y += diff * rotSpeed * deltaTime;
	} else if (hasInput) {
		Vector2 dir = {inputDir.x / inputLength, inputDir.y / inputLength};
		float targetAngle = std::atan2(dir.x, dir.y);
		float diff = targetAngle - rotate.y;

		const float kPi = std::numbers::pi_v<float>;
		while (diff < -kPi)
			diff += 2.0f * kPi;
		while (diff > kPi)
			diff -= 2.0f * kPi;

		rotate.y += diff * rotSpeed * deltaTime;
	}

	// 位置更新
	transform->translate.x += velocity_.x * deltaTime;
	transform->translate.z += velocity_.y * deltaTime;

#ifdef USE_IMGUI
	ImGui::Begin("SpeedParam");
	ImGui::DragFloat("Walk", &maxSpeedWalk_, 0.01f);
	ImGui::DragFloat("Run", &maxSpeedRun_, 0.01f);
	ImGui::DragFloat("Dash", &maxSpeedDash_, 0.01f);
	ImGui::DragFloat("Acceleration", &acceleration_, 1.0f);
	ImGui::DragFloat("Friction", &friction_, 1.0f);
	ImGui::End();
#endif
}