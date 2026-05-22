#pragma once
#include "Vector3.h"
#include "Vector4.h"

struct GrayscaleParam {
	float intensity = 1.0f;
	Vector3 luminanceWeight = {0.2125f, 0.7154f, 0.0721f};
	Vector3 blendColor = {1.0f, 1.0f, 1.0f};
	float blendStrength = 0.0f;
};

struct VignetteParam {
	Vector4 color = {0.0f, 0.0f, 0.0f, 1.0f};
	float intensity = 1.0f;
};

struct SepiaParam {
	float intensity = 1.0f;
	Vector3 sepiaColor = {1.0f, 0.69f, 0.43f};
	float toneStrength = 1.0f;
};

struct SmoothingParam {
	int radius = 2;
	float intensity = 1.0f;
	float texelSize[2] = {0.0f, 0.0f};
};