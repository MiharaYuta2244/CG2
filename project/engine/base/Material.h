#pragma once
#include "Matrix4x4.h"
#include "Vector4.h"
#include <widemath.h>

/// <summary>
/// GPUに送るマテリアルの構造体
/// </summary>
struct Material {
	Vector4 color;
	Matrix4x4 uvTransform;
	int32_t enableLighting;
	int32_t enableFoging;
	float shininess;
	float envScale;
	float time;
	int32_t enableNoise;
};