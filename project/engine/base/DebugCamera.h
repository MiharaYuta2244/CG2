#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include "DirectInput.h"
#include "GamePad.h"

class DebugCamera {
public:
	void Initialize();
	void SetPivot(const Vector3& p);
	void Update(const DirectInput& input, const GamePad& gamePad);
	void UpdateViewMatrix();

	// Getter
	Matrix4x4 GetViewMatrix() { return viewMatrix_; }
	Vector3& GetTranslation() { return translation_; }
	
	// Setter
	void SetTranslation(Vector3 translation) { translation_ = translation; }

private:
	// 累積回転行列
	Matrix4x4 orientation_;
	// ローカル座標
	Vector3 translation_;
	// ビュー行列
	Matrix4x4 viewMatrix_;
	// 射影行列
	Matrix4x4 projectionMatrix_;
	// ピボット
	Vector3 pivot_;
};
