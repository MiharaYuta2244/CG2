#pragma once
#include "Vector4.h"
#include "Matrix4x4.h"

struct ParticleMaterial {
	Vector4 color;
	Matrix4x4 uvTransform;
	float alphaCutoff;
	float padding[3];
};