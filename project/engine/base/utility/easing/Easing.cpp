#include "Easing.h"

// --- Sine ---
float Easing::EaseInSine(float t) { return 1.0f - cos((t * std::numbers::pi_v<float>) / 2.0f); }
float Easing::EaseOutSine(float t) { return sin((t * std::numbers::pi_v<float>) / 2.0f); }
float Easing::EaseInOutSine(float t) { return -(cos(std::numbers::pi_v<float> * t) - 1.0f) / 2.0f; }

// --- Quadratic ---
float Easing::EaseInQuad(float t) { return t * t; }
float Easing::EaseOutQuad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
float Easing::EaseInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f; }

// --- Cubic ---
float Easing::EaseInCubic(float t) { return t * t * t; }
float Easing::EaseOutCubic(float t) { return 1.0f - pow(1.0f - t, 3.0f); }
float Easing::EaseInOutCubic(float t) { return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f; }

// --- Quartic ---
float Easing::EaseInQuart(float t) { return t * t * t * t; }
float Easing::EaseOutQuart(float t) { return 1.0f - pow(1.0f - t, 4.0f); }
float Easing::EaseInOutQuart(float t) { return t < 0.5f ? 8.0f * pow(t, 4.0f) : 1.0f - pow(-2.0f * t + 2.0f, 4.0f) / 2.0f; }

// --- Quintic ---
float Easing::EaseInQuint(float t) { return t * t * t * t * t; }
float Easing::EaseOutQuint(float t) { return 1.0f - pow(1.0f - t, 5.0f); }
float Easing::EaseInOutQuint(float t) { return t < 0.5f ? 16.0f * pow(t, 5.0f) : 1.0f - pow(-2.0f * t + 2.0f, 5.0f) / 2.0f; }

// --- Expo ---
float Easing::EaseInExpo(float t) { return (t == 0.0f) ? 0.0f : pow(2.0f, 10.0f * t - 10.0f); }
float Easing::EaseOutExpo(float t) { return (t == 1.0f) ? 1.0f : 1.0f - pow(2.0f, -10.0f * t); }
float Easing::EaseInOutExpo(float t) {
	if (t == 0.0f)
		return 0.0f;
	if (t == 1.0f)
		return 1.0f;
	return t < 0.5f ? pow(2.0f, 20.0f * t - 10.0f) / 2.0f : (2.0f - pow(2.0f, -20.0f * t + 10.0f)) / 2.0f;
}

// --- Circ ---
float Easing::EaseInCirc(float t) { return 1.0f - sqrt(1.0f - t * t); }
float Easing::EaseOutCirc(float t) { return sqrt(1.0f - pow(t - 1.0f, 2.0f)); }
float Easing::EaseInOutCirc(float t) { return t < 0.5f ? (1.0f - sqrt(1.0f - pow(2.0f * t, 2.0f))) / 2.0f : (sqrt(1.0f - pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f; }

// --- Back ---
float Easing::EaseInBack(float t) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	return c3 * t * t * t - c1 * t * t;
}
float Easing::EaseOutBack(float t) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	return 1.0f + c3 * pow(t - 1.0f, 3.0f) + c1 * pow(t - 1.0f, 2.0f);
}
float Easing::EaseInOutBack(float t) {
	const float c1 = 1.70158f;
	const float c2 = c1 * 1.525f;
	return t < 0.5f ? (pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f : (pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
}

// --- Elastic ---
float Easing::EaseInElastic(float t) {
	const float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;
	return (t == 0.0f) ? 0.0f : (t == 1.0f) ? 1.0f : -pow(2.0f, 10.0f * t - 10.0f) * sin((t * 10.0f - 10.75f) * c4);
}
float Easing::EaseOutElastic(float t) {
	const float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;
	return (t == 0.0f) ? 0.0f : (t == 1.0f) ? 1.0f : pow(2.0f, -10.0f * t) * sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}
float Easing::EaseInOutElastic(float t) {
	const float c5 = (2.0f * std::numbers::pi_v<float>) / 4.5f;
	if (t == 0.0f)
		return 0.0f;
	if (t == 1.0f)
		return 1.0f;
	return t < 0.5f ? -(pow(2.0f, 20.0f * t - 10.0f) * sin((20.0f * t - 11.125f) * c5)) / 2.0f : (pow(2.0f, -20.0f * t + 10.0f) * sin((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
}

// --- Bounce ---
float Easing::EaseOutBounce(float t) {
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (t < 1.0f / d1) {
		return n1 * t * t;
	} else if (t < 2.0f / d1) {
		t -= 1.5f / d1;
		return n1 * t * t + 0.75f;
	} else if (t < 2.5f / d1) {
		t -= 2.25f / d1;
		return n1 * t * t + 0.9375f;
	} else {
		t -= 2.625f / d1;
		return n1 * t * t + 0.984375f;
	}
}

float Easing::EaseInBounce(float t) { return 1.0f - EaseOutBounce(1.0f - t); }

float Easing::EaseInOutBounce(float t) { return t < 0.5f ? (1.0f - EaseOutBounce(1.0f - 2.0f * t)) / 2.0f : (1.0f + EaseOutBounce(2.0f * t - 1.0f)) / 2.0f; }