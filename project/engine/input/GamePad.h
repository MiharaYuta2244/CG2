#pragma once
#include <Xinput.h>

struct GamepadAxes {
	float lx = 0.f, ly = 0.f; // 左スティック
	float rx = 0.f, ry = 0.f; // 右スティック
	float lt = 0.f, rt = 0.f; // トリガー
};

struct GamepadButtons {
	bool a = false, b = false, x = false, y = false;
	bool lb = false, rb = false;
	bool back = false, start = false;
	bool ls = false, rs = false;
	bool dpadUp = false, dpadDown = false, dpadLeft = false, dpadRight = false;
};

struct GamepadState {
	bool connected = false;
	GamepadButtons buttons{};
	GamepadButtons buttonsPressed{};  // 立ち上がり
	GamepadButtons buttonsReleased{}; // 立ち下がり
	GamepadAxes axes{};
};

class GamePad {
public:
	GamePad();
	~GamePad();

	void Update(DWORD padIndex = 0);
	const GamepadState& GetState() const { return state_; }

	// 振動
	void Rumble(float leftMotor, float rightMotor, DWORD padIndex = 0);

private:
	using XInputGetState_t = DWORD(WINAPI*)(DWORD, XINPUT_STATE*);
	using XInputSetState_t = DWORD(WINAPI*)(DWORD, XINPUT_VIBRATION*);

	HMODULE xinput_ = nullptr;
	XInputGetState_t pXInputGetState_ = nullptr;
	XInputSetState_t pXInputSetState_ = nullptr;

	GamepadState state_{};
	GamepadButtons prevButtons_{};

	static float NormalizeThumb(SHORT v, SHORT deadzone);
	static float NormalizeTrigger(BYTE v, BYTE threshold);
	static GamepadButtons DecodeButtons(WORD w);
};
