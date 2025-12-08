#pragma once
#include "Vector4.h"
#include "Vector3.h"

struct PointLight
{
	Vector4 color;
	Vector3 position;
	float intensity;
};