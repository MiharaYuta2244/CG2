#pragma once
#include "Transform.h"
#include "Vector4.h"
#include "Vector2.h"

/// <summary>
/// パーティクルの情報構造体
/// </summary>
struct ParticleState
{
	Transform transform;
	Vector2 uvScroll;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};