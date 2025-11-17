#pragma once
#include "MathUtility.h"
#include "Matrix4x4.h"
#include "Vector3.h"

inline Vector3 operator+(const Vector3& v1, const Vector3& v2) { return MathUtility::Add(v1, v2); }
inline Vector3 operator-(const Vector3& v1, const Vector3& v2) { return MathUtility::Subtract(v1, v2); }
inline Vector3 operator*(float s, const Vector3& v) { return MathUtility::Multiply(s, v); }
inline Vector3 operator*(const Vector3& v, float s) { return s * v; }
inline Vector3 operator/(const Vector3& v, float s) { return MathUtility::Multiply(1.0f / s, v); }

inline Vector3 operator-(const Vector3& v) { return {-v.x, -v.y, -v.z}; }
inline Vector3 operator+(const Vector3& v) { return v; }

inline Vector3& operator+=(Vector3& lhs, const Vector3& rhs) {
	lhs = MathUtility::Add(lhs, rhs);
	return lhs;
}

inline Vector3& operator-=(Vector3& lhs, const Vector3& rhs) {
	lhs = MathUtility::Subtract(lhs, rhs);
	return lhs;
}

inline Vector3& operator*=(Vector3& lhs, float s) {
	lhs = MathUtility::Multiply(s, lhs);
	return lhs;
}

inline Vector3& operator/=(Vector3& lhs, float s) {
	lhs = MathUtility::Multiply(1.0f / s, lhs);
	return lhs;
}

inline Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2) { return MathUtility::Add(m1, m2); }
inline Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2) { return MathUtility::Subtract(m1, m2); }
inline Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) { return MathUtility::Multiply(m1, m2); }