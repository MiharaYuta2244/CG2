#pragma once
#include "GameTimer.h"

/// <summary>
/// プレイヤー死亡時、カメラの演出用クラス
/// </summary>
class CameraDeathZoomController {
public:
	// 演出の開始処理
	void Start(float startPosY);

	// カメラのY座標を更新して返す
	float Update(float deltaTime);

	// アクティブ状態かどうか
	bool GetIsActive() const { return isActive_; }

	// 演出が終了したかどうか
	bool GetIsFinished()const {return isFinished_; }

private:
	// 切り替わる回数
	int maxStep_ = 3;

	// 現在の段階
	int step_ = 0;

	// Y座標の変化量
	float addAmount_ = 100.0f;

	// 段階が切り替わる時間
	GameTimer changeTimer_;

	// 現在のY座標
	float currentY_ = 0.0f;

	// アクティブフラグ
	bool isActive_ = false;

	// 終了フラグ
	bool isFinished_ = false;

	// 切り替わる間隔
	float changeDuration_ = 1.0f;
};
