#pragma once
#include "Vector2.h"
#include "Vector4.h"

struct GrayscaleParam {
	float intensity = 0.0f;
};

struct VignetteParam {
	Vector4 color = {0.0f, 0.0f, 0.0f, 1.0f};
	float intensity = 1.0f;
};

struct SepiaParam {
	float intenisity = 1.0f;
};

struct SmoothingParam{
	int radius = 2;
	float intensity = 1.0f;
	float texelSize[2] = {0.0f, 0.0f};
};