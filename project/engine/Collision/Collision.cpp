#include "Collision.h"
#include "MathUtility.h"
#include <algorithm>
#include <cmath>

bool Collision::Intersect(const Sphere& s1, const Sphere& s2) {
	float distanceX = s1.center.x - s2.center.x;
	float distanceY = s1.center.y - s2.center.y;
	float distanceZ = s1.center.z - s2.center.z;
	float distance = std::sqrtf(distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ);

	if (distance <= s1.radius + s2.radius) {
		return true;
	}
	return false;
}

bool Collision::Intersect(const AABB& aabb1, const AABB& aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && // x軸
	    (aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && // y軸
	    (aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) { // z軸
		// 衝突
		return true;
	}

	// 衝突なし
	return false;
}

bool Collision::Intersect(const AABB& aabb, const Sphere& sphere) {
	// 最近接点を求める
	Vector3 closestPoint{std::clamp(sphere.center.x, aabb.min.x, aabb.max.x), std::clamp(sphere.center.y, aabb.min.y, aabb.max.y), std::clamp(sphere.center.z, aabb.min.z, aabb.max.z)};

	// 最近接点との距離を求める
	float distance = MathUtility::Length(MathUtility::Subtract(closestPoint, sphere.center));

	// 距離が半径よりも小さければ衝突
	if (distance <= sphere.radius) {
		// 衝突
		return true;
	}

	// 衝突なし
	return false;
}

bool Collision::Intersect(const AABB& aabb, const Vector3& point) {
	return point.x >= aabb.min.x && point.x <= aabb.max.x && point.y >= aabb.min.y && point.y <= aabb.max.y && point.z >= aabb.min.z && point.z <= aabb.max.z;
}
