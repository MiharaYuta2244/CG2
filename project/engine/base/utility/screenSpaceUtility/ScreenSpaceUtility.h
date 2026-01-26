#pragma once
#include "DebugCamera.h"
#include "Vector2.h"
#include "Vector3.h"

class ScreenSpaceUtility {
public:
	Vector2 WorldToScreen(const Vector3& worldPos, const Vector2& margin = {0, 0}) const;

	void SetCamera(DebugCamera* camera) { camera_ = camera; }

private:
	DebugCamera* camera_ = nullptr;
};
