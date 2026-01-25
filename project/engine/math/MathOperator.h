#pragma once
#include "MathUtility.h"
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

inline Vector2 operator+(const Vector2& v1, const Vector2& v2) { return MathUtility::Add(v1, v2); }

inline Vector2 operator-(const Vector2& v1, const Vector2& v2) { return MathUtility::Subtract(v1, v2); }

inline Vector2 operator*(float s, const Vector2& v) { return MathUtility::Multiply(s, v); }

inline Vector2 operator*(const Vector2& v, float s) { return s * v; }

inline Vector2 operator/(const Vector2& v, float s) { return MathUtility::Multiply(1.0f / s, v); }

inline Vector2 operator-(const Vector2& v) { return {-v.x, -v.y}; }

inline Vector2 operator+(const Vector2& v) { return v; }

inline Vector2& operator+=(Vector2& lhs, const Vector2& rhs) {
	lhs = MathUtility::Add(lhs, rhs);
	return lhs;
}

inline Vector2& operator-=(Vector2& lhs, const Vector2& rhs) {
	lhs = MathUtility::Subtract(lhs, rhs);
	return lhs;
}

inline Vector2& operator*=(Vector2& lhs, float s) {
	lhs = MathUtility::Multiply(s, lhs);
	return lhs;
}

inline Vector2& operator/=(Vector2& lhs, float s) {
	lhs = MathUtility::Multiply(1.0f / s, lhs);
	return lhs;
}

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

inline Vector4 operator+(const Vector4& v1, const Vector4& v2) { return MathUtility::Add(v1, v2); }
inline Vector4 operator-(const Vector4& v1, const Vector4& v2) { return MathUtility::Subtract(v1, v2); }
inline Vector4 operator*(float s, const Vector4& v) { return MathUtility::Multiply(s, v); }
inline Vector4 operator*(const Vector4& v, float s) { return s * v; }
inline Vector4 operator/(const Vector4& v, float s) { return MathUtility::Multiply(1.0f / s, v); }
inline Vector4 operator-(const Vector4& v) { return {-v.x, -v.y, -v.z, -v.w}; }
inline Vector4 operator+(const Vector4& v) { return v; }

inline Vector4& operator+=(Vector4& lhs, const Vector4& rhs) {
	lhs = MathUtility::Add(lhs, rhs);
	return lhs;
}

inline Vector4& operator-=(Vector4& lhs, const Vector4& rhs) {
	lhs = MathUtility::Subtract(lhs, rhs);
	return lhs;
}

inline Vector4& operator*=(Vector4& lhs, float s) {
	lhs = MathUtility::Multiply(s, lhs);
	return lhs;
}

inline Vector4& operator/=(Vector4& lhs, float s) {
	lhs = MathUtility::Multiply(1.0f / s, lhs);
	return lhs;
}

inline Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2) { return MathUtility::Add(m1, m2); }
inline Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2) { return MathUtility::Subtract(m1, m2); }
inline Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) { return MathUtility::Multiply(m1, m2); }