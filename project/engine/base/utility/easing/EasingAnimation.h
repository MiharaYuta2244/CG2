#pragma once
#include "EaseType.h"
#include "Easing.h"
#include "MathOperator.h"
#include "Vector3.h"
#include <algorithm>

template<typename T> class EasingAnimation {
public:
	EasingAnimation() : isActive_(false) {}

	EasingAnimation(T start, T end, float duration, EaseType easeType) : start_(start), end_(end), duration_(duration), time_(0.0f), easeType_(easeType), isActive_(true) {}

	bool Update(float deltaTime, T& outValue) {
		if (!isActive_)
			return false;

		time_ += deltaTime;
		float t = std::min(time_ / duration_, 1.0f);
		float eased = Easing::ApplyEasing(easeType_, t);
		outValue = Interpolate(start_, end_, eased);

		// 指定した時間が経過したらフラグをおろす
		if (time_ >= duration_) {
			isActive_ = false;
		}

		return isActive_;
	}

	// 型ごとの補間
	T Interpolate(const T& a, const T& b, float t) { return a + (b - a) * t; }

	// Getter
	bool GetIsActive() const { return isActive_; }

private:
	T start_;           // 始点
	T end_;             // 終点
	float duration_;    // アニメーション時間
	float time_;        // 経過時間
	EaseType easeType_; // イージングのタイプ
	bool isActive_;      // アクティブかどうか
};
