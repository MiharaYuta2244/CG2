#pragma once
#include "EaseType.h"
#include <cmath>
#include <numbers>

using EaseFunc = float (*)(float);

class Easing {
public:
	// Sine
	static float EaseInSine(float t);
	static float EaseOutSine(float t);
	static float EaseInOutSine(float t);

	// Quadratic
	static float EaseInQuad(float t);
	static float EaseOutQuad(float t);
	static float EaseInOutQuad(float t);

	// Cubic
	static float EaseInCubic(float t);
	static float EaseOutCubic(float t);
	static float EaseInOutCubic(float t);

	// Quartic
	static float EaseInQuart(float t);
	static float EaseOutQuart(float t);
	static float EaseInOutQuart(float t);

	// Quintic
	static float EaseInQuint(float t);
	static float EaseOutQuint(float t);
	static float EaseInOutQuint(float t);

	// Expo
	static float EaseInExpo(float t);
	static float EaseOutExpo(float t);
	static float EaseInOutExpo(float t);

	// Circ
	static float EaseInCirc(float t);
	static float EaseOutCirc(float t);
	static float EaseInOutCirc(float t);

	// Back
	static float EaseInBack(float t);
	static float EaseOutBack(float t);
	static float EaseInOutBack(float t);

	// Elastic
	static float EaseInElastic(float t);
	static float EaseOutElastic(float t);
	static float EaseInOutElastic(float t);

	// Bounce
	static float EaseOutBounce(float t);
	static float EaseInBounce(float t);
	static float EaseInOutBounce(float t);

	static inline float ApplyEasing(EaseType type, float t) {
		return EaseTable[static_cast<int>(type)](t);
	}

private:
	static constexpr EaseFunc EaseTable[] = {
	    &Easing::EaseInSine,    &Easing::EaseOutSine,    &Easing::EaseInOutSine,    &Easing::EaseInQuad,    &Easing::EaseOutQuad,  &Easing::EaseInOutQuad,
	    &Easing::EaseInCubic,   &Easing::EaseOutCubic,   &Easing::EaseInOutCubic,   &Easing::EaseInQuart,   &Easing::EaseOutQuart, &Easing::EaseInOutQuart,
	    &Easing::EaseInQuint,   &Easing::EaseOutQuint,   &Easing::EaseInOutQuint,   &Easing::EaseInExpo,    &Easing::EaseOutExpo,  &Easing::EaseInOutExpo,
	    &Easing::EaseInCirc,    &Easing::EaseOutCirc,    &Easing::EaseInOutCirc,    &Easing::EaseInBack,    &Easing::EaseOutBack,  &Easing::EaseInOutBack,
	    &Easing::EaseInElastic, &Easing::EaseOutElastic, &Easing::EaseInOutElastic, &Easing::EaseOutBounce, &Easing::EaseInBounce, &Easing::EaseInOutBounce,
	};
};