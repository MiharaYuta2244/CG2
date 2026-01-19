#pragma once
#include "EasingAnimation.h"

template<typename T> struct AnimationBundle {
	EasingAnimation<T> anim; // アニメーション本体
	T start;                 // 開始値
	T end;                   // 目標値
	T temp;                  // 一時値
};
