#pragma once
#include "Transform.h"

struct ParticleState
{
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};