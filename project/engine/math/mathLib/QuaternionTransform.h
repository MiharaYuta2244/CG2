#pragma once
#include "Quaternion.h"
#include "Vector3.h"

struct QuaternionTransform {
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;
};