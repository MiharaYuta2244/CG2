#pragma once
#include "Vector4.h"

class ColorPalette {
public:
	// -----------------------------
	// 基本色
	// -----------------------------
	static constexpr Vector4 White() { return {1, 1, 1, 1}; }
	static constexpr Vector4 Black() { return {0, 0, 0, 1}; }
	static constexpr Vector4 Red() { return {1, 0, 0, 1}; }
	static constexpr Vector4 Green() { return {0, 1, 0, 1}; }
	static constexpr Vector4 Blue() { return {0, 0, 1, 1}; }
	static constexpr Vector4 Yellow() { return {1, 1, 0, 1}; }
	static constexpr Vector4 Cyan() { return {0, 1, 1, 1}; }
	static constexpr Vector4 Magenta() { return {1, 0, 1, 1}; }

	// -----------------------------
	// グレー系
	// -----------------------------
	static constexpr Vector4 Gray() { return {0.5f, 0.5f, 0.5f, 1}; }
	static constexpr Vector4 LightGray() { return {0.75f, 0.75f, 0.75f, 1}; }
	static constexpr Vector4 DarkGray() { return {0.25f, 0.25f, 0.25f, 1}; }

	// -----------------------------
	// UIでよく使う色
	// -----------------------------
	static constexpr Vector4 Transparent() { return {0, 0, 0, 0}; }
	static constexpr Vector4 UIHighlight() { return {0.2f, 0.6f, 1.0f, 1}; }
	static constexpr Vector4 UISuccess() { return {0.2f, 0.8f, 0.2f, 1}; }
	static constexpr Vector4 UIWarning() { return {1.0f, 0.8f, 0.2f, 1}; }
	static constexpr Vector4 UIError() { return {1.0f, 0.2f, 0.2f, 1}; }

	// -----------------------------
	// パステルカラー
	// -----------------------------
	static constexpr Vector4 PastelRed() { return {1.0f, 0.6f, 0.6f, 1}; }
	static constexpr Vector4 PastelGreen() { return {0.6f, 1.0f, 0.6f, 1}; }
	static constexpr Vector4 PastelBlue() { return {0.6f, 0.6f, 1.0f, 1}; }
	static constexpr Vector4 PastelYellow() { return {1.0f, 1.0f, 0.6f, 1}; }
	static constexpr Vector4 PastelPurple() { return {0.8f, 0.6f, 1.0f, 1}; }
	static constexpr Vector4 PastelPink() { return {1.0f, 0.6f, 0.8f, 1}; }

	// -----------------------------
	// ダークテーマ向け
	// -----------------------------
	static constexpr Vector4 DarkRed() { return {0.5f, 0.0f, 0.0f, 1}; }
	static constexpr Vector4 DarkGreen() { return {0.0f, 0.5f, 0.0f, 1}; }
	static constexpr Vector4 DarkBlue() { return {0.0f, 0.0f, 0.5f, 1}; }
	static constexpr Vector4 DarkYellow() { return {0.5f, 0.5f, 0.0f, 1}; }
	static constexpr Vector4 DarkCyan() { return {0.0f, 0.5f, 0.5f, 1}; }
	static constexpr Vector4 DarkMagenta() { return {0.5f, 0.0f, 0.5f, 1}; }

	// -----------------------------
	// ネオンカラー（エフェクト向け）
	// -----------------------------
	static constexpr Vector4 NeonRed() { return {1.0f, 0.2f, 0.2f, 1}; }
	static constexpr Vector4 NeonGreen() { return {0.2f, 1.0f, 0.2f, 1}; }
	static constexpr Vector4 NeonBlue() { return {0.2f, 0.2f, 1.0f, 1}; }
	static constexpr Vector4 NeonPink() { return {1.0f, 0.2f, 0.6f, 1}; }
	static constexpr Vector4 NeonPurple() { return {0.6f, 0.2f, 1.0f, 1}; }

	// -----------------------------
	// メタリック系（UI/エフェクト向け）
	// -----------------------------
	static constexpr Vector4 Gold() { return {1.0f, 0.84f, 0.0f, 1}; }
	static constexpr Vector4 Silver() { return {0.75f, 0.75f, 0.75f, 1}; }
	static constexpr Vector4 Bronze() { return {0.8f, 0.5f, 0.2f, 1}; }

	// -----------------------------
	// 便利関数
	// -----------------------------
	static constexpr Vector4 AddAlpha(const Vector4& c, float a) { return {c.x, c.y, c.z, a}; }

	static constexpr Vector4 Multiply(const Vector4& c, float s) { return {c.x * s, c.y * s, c.z * s, c.w}; }

	static constexpr Vector4 Lerp(const Vector4& a, const Vector4& b, float t) { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t}; }

	// HSV → RGB変換
	static Vector4 HSV(float h, float s, float v, float a = 1.0f) {
		float c = v * s;
		float x = c * (1 - std::fabsf(std::fmodf(h * 6, 2) - 1));
		float m = v - c;

		float r, g, b;
		if (h < 1 / 6.f) {
			r = c;
			g = x;
			b = 0;
		} else if (h < 2 / 6.f) {
			r = x;
			g = c;
			b = 0;
		} else if (h < 3 / 6.f) {
			r = 0;
			g = c;
			b = x;
		} else if (h < 4 / 6.f) {
			r = 0;
			g = x;
			b = c;
		} else if (h < 5 / 6.f) {
			r = x;
			g = 0;
			b = c;
		} else {
			r = c;
			g = 0;
			b = x;
		}

		return {r + m, g + m, b + m, a};
	}
};
