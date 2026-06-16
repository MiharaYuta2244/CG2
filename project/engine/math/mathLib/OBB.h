#pragma once
#include "Vector3.h"

struct OBB {
	Vector3 center;
	Vector3 orientations[3];
	Vector3 size;
};