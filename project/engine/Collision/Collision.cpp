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
bool Collision::Intersect(const OBB& obb1, const OBB& obb2) {
	// テストする15本の分離軸
	Vector3 axes[15];

	// 1-3: OBB1のローカル軸
	axes[0] = obb1.orientations[0];
	axes[1] = obb1.orientations[1];
	axes[2] = obb1.orientations[2];

	// 4-6: OBB2のローカル軸
	axes[3] = obb2.orientations[0];
	axes[4] = obb2.orientations[1];
	axes[5] = obb2.orientations[2];

	// 7-15: 両者のローカル軸同士の外積
	int axisIndex = 6;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			axes[axisIndex++] = MathUtility::Cross(obb1.orientations[i], obb2.orientations[j]);
		}
	}

	// OBB間の中心距離ベクトル
	Vector3 distanceVec = MathUtility::Subtract(obb2.center, obb1.center);

	// 15本の軸すべてに対して分離軸テストを実行
	for (int i = 0; i < 15; ++i) {
		// 軸の長さの2乗を計算 (平行な軸の外積などでゼロベクトルになるケースを除外)
		float lengthSq = axes[i].x * axes[i].x + axes[i].y * axes[i].y + axes[i].z * axes[i].z;
		if (lengthSq < 1e-6f) {
			continue; // 軸が無効な場合はスキップ
		}

		// 中心間の距離を軸に投影
		float dist = std::abs(MathUtility::Dot(distanceVec, axes[i]));

		// 各OBBの軸への投影半径を計算
		float r1 = obb1.size.x * std::abs(MathUtility::Dot(axes[i], obb1.orientations[0])) + obb1.size.y * std::abs(MathUtility::Dot(axes[i], obb1.orientations[1])) +
		           obb1.size.z * std::abs(MathUtility::Dot(axes[i], obb1.orientations[2]));

		float r2 = obb2.size.x * std::abs(MathUtility::Dot(axes[i], obb2.orientations[0])) + obb2.size.y * std::abs(MathUtility::Dot(axes[i], obb2.orientations[1])) +
		           obb2.size.z * std::abs(MathUtility::Dot(axes[i], obb2.orientations[2]));

		// 分離軸が見つかった場合、衝突していない
		if (dist > r1 + r2) {
			return false;
		}
	}

	// すべての軸で重なっていれば衝突している
	return true;
}

bool Collision::Intersect(const AABB& aabb, const OBB& obb) {
	// AABBをOBBの形式に変換する
	OBB aabbAsObb;

	// 中心の計算
	aabbAsObb.center.x = (aabb.min.x + aabb.max.x) * 0.5f;
	aabbAsObb.center.y = (aabb.min.y + aabb.max.y) * 0.5f;
	aabbAsObb.center.z = (aabb.min.z + aabb.max.z) * 0.5f;

	// AABBの軸はワールド座標軸に一致
	aabbAsObb.orientations[0] = {1.0f, 0.0f, 0.0f};
	aabbAsObb.orientations[1] = {0.0f, 1.0f, 0.0f};
	aabbAsObb.orientations[2] = {0.0f, 0.0f, 1.0f};

	// 半値幅の計算
	aabbAsObb.size.x = (aabb.max.x - aabb.min.x) * 0.5f;
	aabbAsObb.size.y = (aabb.max.y - aabb.min.y) * 0.5f;
	aabbAsObb.size.z = (aabb.max.z - aabb.min.z) * 0.5f;

	// OBB同士の判定に委譲
	return Intersect(aabbAsObb, obb);
}

bool Collision::Intersect(const OBB& obb, const AABB& aabb) { return Intersect(aabb, obb); }

bool Collision::Intersect(const Segment& segment, const AABB& aabb, float& outT) {
	float tMin = 0.0f;
	float tMax = 1.0f;

	// X軸
	if (std::abs(segment.diff.x) < 1e-6f) {
		if (segment.origin.x < aabb.min.x || segment.origin.x > aabb.max.x)
			return false;
	} else {
		float invD = 1.0f / segment.diff.x;
		float t0 = (aabb.min.x - segment.origin.x) * invD;
		float t1 = (aabb.max.x - segment.origin.x) * invD;
		if (invD < 0.0f)
			std::swap(t0, t1);
		tMin = std::max(tMin, t0);
		tMax = std::min(tMax, t1);
		if (tMax < tMin)
			return false;
	}

	// Y軸
	if (std::abs(segment.diff.y) < 1e-6f) {
		if (segment.origin.y < aabb.min.y || segment.origin.y > aabb.max.y)
			return false;
	} else {
		float invD = 1.0f / segment.diff.y;
		float t0 = (aabb.min.y - segment.origin.y) * invD;
		float t1 = (aabb.max.y - segment.origin.y) * invD;
		if (invD < 0.0f)
			std::swap(t0, t1);
		tMin = std::max(tMin, t0);
		tMax = std::min(tMax, t1);
		if (tMax < tMin)
			return false;
	}

	// Z軸
	if (std::abs(segment.diff.z) < 1e-6f) {
		if (segment.origin.z < aabb.min.z || segment.origin.z > aabb.max.z)
			return false;
	} else {
		float invD = 1.0f / segment.diff.z;
		float t0 = (aabb.min.z - segment.origin.z) * invD;
		float t1 = (aabb.max.z - segment.origin.z) * invD;
		if (invD < 0.0f)
			std::swap(t0, t1);
		tMin = std::max(tMin, t0);
		tMax = std::min(tMax, t1);
		if (tMax < tMin)
			return false;
	}

	outT = tMin;
	return true;
}