#pragma once
#include "DirectInput.h"
#include "GamePad.h"
#include "MathUtility.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include <Transform.h>

/// <summary>
/// カメラのクラス
/// </summary>
class Camera {
public:
	Camera();
	void Initialize();
	void SetPivot(const Vector3& p);
	void Update(const DirectInput& input, const GamePad& gamePad);
	void UpdateViewMatrix();

	// Getter
	Matrix4x4& GetWorldMatrix() { return worldMatrix_; }
	Matrix4x4 GetViewMatrix() { return viewMatrix_; }
	Matrix4x4& GetProjection() { return projectionMatrix_; }
	Matrix4x4& GetViewProjectionMatrix() { return viewProjectionMatrix_; }
	Vector3& GetTranslation() { return transform_.translate; }
	Vector3& GetRotate() { return transform_.rotate; }
	const Vector3& GetEuler() const { return euler_; }

	// Setter
	void SetTranslation(Vector3 translation) { transform_.translate = translation; }
	void SetRotate(const Vector3& rotate);
	void SetFovY(float fovY) { fovY_ = fovY; }
	void SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }
	void SetNearClip(float nearClip) { nearClip_ = nearClip; }
	void SetFarClip(float farClip) { farClip_ = farClip; }
	void SetEuler(const Vector3& euler) {
		euler_ = euler;
		euler_.z = 0.0f;
		UpdateOrientation();
		UpdateViewMatrix();
	}

	// シェイク開始処理
	void StartShake(float duration, float magnitude);

	// シェイクの更新処理
	void ShakeCamera(float deltaTime, float shakePower);

private:
	void UpdateOrientation();

private:
	// 累積回転行列
	Matrix4x4 orientation_;
	// ローカル座標
	Transform transform_;
	// ビュー行列
	Matrix4x4 viewMatrix_;
	// 射影行列
	Matrix4x4 projectionMatrix_;
	// ピボット
	Vector3 pivot_;
	// ワールド座標
	Matrix4x4 worldMatrix_;
	// 水平方向視野角
	float fovY_;
	// アスペクト比
	float aspectRatio_;
	// ニアクリップ距離
	float nearClip_;
	// ファークリップ距離
	float farClip_;
	// ビュープロジェクション行列
	Matrix4x4 viewProjectionMatrix_;
	// オイラー
	Vector3 euler_;

	// ===================================
	// シェイク用の変数
	// ===================================
	bool isShake_ = false;
	float shakeDuration_;
	float shakeTimer_ = 0;
	float magnitude_;
	Vector3 shakeOffset_ = {0.0f, 0.0f, 0.0f};
};
