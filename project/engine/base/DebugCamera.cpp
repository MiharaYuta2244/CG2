#include "DebugCamera.h"
#include "MathUtility.h"
#include "WinApp.h"
#include <numbers>

DebugCamera::DebugCamera()
    : transform_({
          {1.0f, 1.0f, -50.0f},
          {0.0f, 0.0f, 0.0f},
          {0.0f, 0.0f, 0.0f}
}),
      fovY_(0.45f), aspectRatio_(float(WinApp::kClientWidth) / float(WinApp::kClientHeight)), nearClip_(0.1f), farClip_(1000.0f),
      worldMatrix_(MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate)), viewMatrix_(MathUtility::Inverse(worldMatrix_)),
      projectionMatrix_(MathUtility::MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_)), viewProjectionMatrix_(MathUtility::Multiply(viewMatrix_, projectionMatrix_)),
      orientation_(MathUtility::MakeIdentity4x4()), pivot_({0.0f, 0.0f, 0.0f}) {}

void DebugCamera::Initialize() {
	UpdateViewMatrix();
}

void DebugCamera::SetPivot(const Vector3& p) {
	Vector3 offset = MathUtility::Subtract(transform_.translate, pivot_);
	pivot_ = p;
	transform_.translate = MathUtility::Add(pivot_, offset);
}

void DebugCamera::Update(const DirectInput& input, const GamePad& gamePad) {
	// フリー回転
	if (input.MouseButtonDown(1)) {
		float dx = input.GetMouseDeltaX() * 0.001f;
		float dy = input.GetMouseDeltaY() * 0.001f;

		Matrix4x4 yaw = MathUtility::MakeYawRotateMatrix(dx);
		Matrix4x4 pitch = MathUtility::MakePitchRotateMatrix(dy);

		orientation_ = MathUtility::Multiply(pitch, MathUtility::Multiply(yaw, orientation_));
		orientation_ = MathUtility::Orthonormalize(orientation_);
	}

	// ホイールドラッグ: 原点中心の回転
	if (!input.KeyDown(DIK_LSHIFT) && input.MouseButtonDown(2)) {
		float dx = input.GetMouseDeltaX() * 0.001f;
		float dy = input.GetMouseDeltaY() * 0.001f;

		Matrix4x4 yaw = MathUtility::MakeYawRotateMatrix(dx);
		Matrix4x4 pitch = MathUtility::MakePitchRotateMatrix(dy);
		Matrix4x4 rot = MathUtility::Multiply(pitch, yaw);

		// 原点中心の回転（pivotを使わず、transformの位置を直接回転）
		Vector3 pos = transform_.translate;
		pos = MathUtility::MultiplyVector(pos, rot);
		transform_.translate = pos;

		orientation_ = MathUtility::Multiply(rot, orientation_);
		orientation_ = MathUtility::Orthonormalize(orientation_);
	}

	auto MoveLocal = [&](const Vector3& local) {
		Vector3 world = MathUtility::MultiplyVector(local, orientation_);
		transform_.translate = MathUtility::Add(transform_.translate, world);
		pivot_ = MathUtility::Add(pivot_, world);
	};

	const float speed = 0.8f;

	// マウスホイール回転で前後移動
	float wheelDelta = input.GetMouseWheel();
	if (wheelDelta != 0.0f) {
		float moveSpeed = wheelDelta > 0.0f ? speed : -speed;
		MoveLocal({0.0f, 0.0f, moveSpeed});
	}

	// Shift + ホイールドラッグ中のマウス移動で上下左右移動（Blenderスタイル）
	if (input.KeyDown(DIK_LSHIFT) && input.MouseButtonDown(2)) {
		float dx = input.GetMouseDeltaX() * 0.1f;
		float dy = input.GetMouseDeltaY() * 0.1f;

		// マウス移動量に応じた連続的な移動
		Vector3 moveVec = {-dx, dy, 0.0f};  // マウスX移動で左右、Y移動で上下
		Vector3 world = MathUtility::MultiplyVector(moveVec, orientation_);
		transform_.translate = MathUtility::Add(transform_.translate, world);
		pivot_ = MathUtility::Add(pivot_, world);
	}

	UpdateViewMatrix();
}

void DebugCamera::UpdateViewMatrix() {
	worldMatrix_ = MathUtility::Multiply(orientation_, MathUtility::MakeTranslateMatrix(transform_.translate));
	viewMatrix_ = MathUtility::Inverse(worldMatrix_);
	projectionMatrix_ = MathUtility::MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = MathUtility::Multiply(viewMatrix_, projectionMatrix_);
}
