#pragma once
#include "AABB.h"
#include "Sphere.h"
namespace Collision {

// 汎用テンプレート
template<typename T1, typename T2> bool Intersect(const T1& a, const T2& b) {
	static_assert(sizeof(T1) == 0, "IsCollision not implemented for these types.");
	return false;
}

bool Intersect(const Sphere& s1, const Sphere& s2);
bool Intersect(const AABB& aabb1, const AABB& aabb2);
bool Intersect(const AABB& aabb, const Sphere& sphere);
bool Intersect(const AABB& aabb, const Vector3& point);
}; // namespace Collision
