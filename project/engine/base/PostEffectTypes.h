#pragma once
#include "Matrix4x4.h"
#include "Vector2.h"
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

struct GaussianParam {
	int radius = 2;
	float intensity = 1.0f;
	float sigma = 2.0f;
	float padding0;
	float texelSize[2] = {0.0f, 0.0f};
	float paddeing1[2];
};

struct DepthOutlineParam {
	Matrix4x4 projectionInverse;
};

struct RadialBlurParam {
	Vector2 center = {0.5f, 0.5f};
	float blurWidth = 0.01f;
	float numSamples = 1.0f;
};

struct DissolveParam {
	float threshold = 1.0f;
	Vector3 edgeColor = {1.0f, 0.4f, 0.3f};
};

struct RandomParam{
	float time = 0.0f;
	float padding[3];
};

struct GlitchParam {
	float time = 0.0f;
	float intensity = 0.0f;
	float padding[2];
};

struct ScanlineParam {
	float scanlineCount = 400.0f;
	float intensity = 0.5f;
	float speed = 1.0f;
	float time = 0.0f;
};

struct DistortionParam {
	float time = 0.0f;
	float speed = 2.0f;
	float amplitude = 0.01f;
	float frequency = 15.0f;
};

struct BarrelDistortionParam {
	float strength = 1.0f;
	float padding[3];
};

struct DeathEffectParam {
	float intensity = 0.0f;
	float padding[3];
};