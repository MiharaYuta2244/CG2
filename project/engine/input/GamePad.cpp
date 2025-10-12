#include <windows.h>
#include "GamePad.h"
#include <cmath>

#ifndef HIDWORD
#define HIDWORD(_qw) ((DWORD)(((_qw) >> 32) & 0xffffffff))
#endif

GamePad::GamePad() {
	xinput_ = LoadLibraryW(L"xinput1_4.dll");
	if (!xinput_)
		xinput_ = LoadLibraryW(L"xinput1_3.dll");
	if (xinput_) {
		pXInputGetState_ = (XInputGetState_t)GetProcAddress(xinput_, "XInputGetState");
		pXInputSetState_ = (XInputSetState_t)GetProcAddress(xinput_, "XInputSetState");
	}
}

GamePad::~GamePad() {
	if (xinput_)
		FreeLibrary(xinput_);
}

float GamePad::NormalizeThumb(SHORT v, SHORT deadzone) {
	// deadzoneの外だけを[-1,1]に再マップ
	int sv = (int)v;
	int dz = (int)deadzone;
	if (std::abs(sv) <= dz)
		return 0.0f;

	const float sign = (sv < 0) ? -1.0f : 1.0f;
	const float range = 32767.0f - dz; // SHORTの最大域
	float mag = (std::abs(sv) - dz) / range;
	if (mag > 1.0f)
		mag = 1.0f;
	return mag * sign;
}

float GamePad::NormalizeTrigger(BYTE v, BYTE threshold) {
	if (v <= threshold)
		return 0.0f;
	float t = (v - threshold) / (255.0f - threshold);
	if (t > 1.0f)
		t = 1.0f;
	return t;
}

GamepadButtons GamePad::DecodeButtons(WORD w) {
	GamepadButtons b{};
	b.a = (w & XINPUT_GAMEPAD_A) != 0;
	b.b = (w & XINPUT_GAMEPAD_B) != 0;
	b.x = (w & XINPUT_GAMEPAD_X) != 0;
	b.y = (w & XINPUT_GAMEPAD_Y) != 0;
	b.lb = (w & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
	b.rb = (w & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
	b.back = (w & XINPUT_GAMEPAD_BACK) != 0;
	b.start = (w & XINPUT_GAMEPAD_START) != 0;
	b.ls = (w & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
	b.rs = (w & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
	b.dpadUp = (w & XINPUT_GAMEPAD_DPAD_UP) != 0;
	b.dpadDown = (w & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
	b.dpadLeft = (w & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
	b.dpadRight = (w & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
	return b;
}

static GamepadButtons DiffPressed(const GamepadButtons& now, const GamepadButtons& prev) {
	GamepadButtons out;
#define EDGE(field) out.field = (now.field && !prev.field)
	EDGE(a);
	EDGE(b);
	EDGE(x);
	EDGE(y);
	EDGE(lb);
	EDGE(rb);
	EDGE(back);
	EDGE(start);
	EDGE(ls);
	EDGE(rs);
	EDGE(dpadUp);
	EDGE(dpadDown);
	EDGE(dpadLeft);
	EDGE(dpadRight);
#undef EDGE
	return out;
}

static GamepadButtons DiffReleased(const GamepadButtons& now, const GamepadButtons& prev) {
	GamepadButtons out;
#define EDGE(field) out.field = (!now.field && prev.field)
	EDGE(a);
	EDGE(b);
	EDGE(x);
	EDGE(y);
	EDGE(lb);
	EDGE(rb);
	EDGE(back);
	EDGE(start);
	EDGE(ls);
	EDGE(rs);
	EDGE(dpadUp);
	EDGE(dpadDown);
	EDGE(dpadLeft);
	EDGE(dpadRight);
#undef EDGE
	return out;
}

void GamePad::Update(DWORD padIndex) {
	GamepadState out{};
	if (!pXInputGetState_) {
		state_ = out;
		return;
	}

	XINPUT_STATE xi{};
	DWORD dw = pXInputGetState_(padIndex, &xi);
	if (dw != ERROR_SUCCESS) {
		// 未接続
		out.connected = false;
		prevButtons_ = {}; // 接続切れ時は前回情報をクリア
		state_ = out;
		return;
	}

	out.connected = true;
	// ボタン
	auto btn = DecodeButtons(xi.Gamepad.wButtons);
	out.buttons = btn;
	out.buttonsPressed = DiffPressed(btn, prevButtons_);
	out.buttonsReleased = DiffReleased(btn, prevButtons_);
	prevButtons_ = btn;

	// スティック
	constexpr SHORT DZ_L = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;  // 7849
	constexpr SHORT DZ_R = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE; // 8689
	out.axes.lx = NormalizeThumb(xi.Gamepad.sThumbLX, DZ_L);
	out.axes.ly = -NormalizeThumb(xi.Gamepad.sThumbLY, DZ_L); // 画面座標に合わせYを反転するならマイナス
	out.axes.rx = NormalizeThumb(xi.Gamepad.sThumbRX, DZ_R);
	out.axes.ry = -NormalizeThumb(xi.Gamepad.sThumbRY, DZ_R);

	// トリガー
	constexpr BYTE TH_T = XINPUT_GAMEPAD_TRIGGER_THRESHOLD; // 30
	out.axes.lt = NormalizeTrigger(xi.Gamepad.bLeftTrigger, TH_T);
	out.axes.rt = NormalizeTrigger(xi.Gamepad.bRightTrigger, TH_T);

	state_ = out;
}

void GamePad::Rumble(float leftMotor, float rightMotor, DWORD padIndex) {
	if (!pXInputSetState_)
		return;
	auto clamp = [](float v) { return (v < 0.f) ? 0.f : (v > 1.f ? 1.f : v); };
	XINPUT_VIBRATION vib{};
	vib.wLeftMotorSpeed = static_cast<WORD>(clamp(leftMotor) * 65535.0f);
	vib.wRightMotorSpeed = static_cast<WORD>(clamp(rightMotor) * 65535.0f);
	pXInputSetState_(padIndex, &vib);
}