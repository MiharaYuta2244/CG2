#include "CameraDeathZoomController.h"

void CameraDeathZoomController::Start(float startPosY) {
	if (isActive_)
		return;

	currentY_ = startPosY;
	step_ = 0;
	changeTimer_.Initialize(changeDuration_);
	isActive_ = true;
}

float CameraDeathZoomController::Update(float deltaTime) {
	if (step_ >= maxStep_) {
		isFinished_ = true;
		return currentY_;
	}

	// タイマーの更新
	changeTimer_.Update(deltaTime);

	// タイマーが終了したら
	if (changeTimer_.IsEnd()) {
		// Y座標に加算
		currentY_ += addAmount_;

		// 段階数増加
		step_++;

		// タイマーの初期化
		changeTimer_.Initialize(changeDuration_);
	}

	return currentY_;
}