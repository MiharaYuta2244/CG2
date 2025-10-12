#pragma once
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include "MouseState.h"
#include <dinput.h>
#include <functional>
#include <memory>
#include <wrl.h>
#include <WinApp.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

class DirectInput {
public:
	// namespace省略
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	~DirectInput();
	void Initialize(WinApp* winApp);
	void Update();

	// キーボード
	bool KeyDown(uint8_t keyCode) const;
	bool KeyTriggered(uint8_t keyCode) const;
	bool KeyReleased(uint8_t keyCode) const;

	// マウス
	bool MouseButtonDown(uint8_t button) const;
	bool MouseButtonTriggered(uint8_t button) const;
	bool MouseButtonReleased(uint8_t button) const;
	float GetMouseDeltaX() const { return static_cast<float>(mouseState_.lX); }
	float GetMouseDeltaY() const { return static_cast<float>(mouseState_.lY); }
	float GetMouseWheel() const { return static_cast<float>(mouseState_.lZ); }

private:
	ComPtr<IDirectInput8> directInput_;
	ComPtr<IDirectInputDevice8> keyboard_;

	// キーボード
	uint8_t key_[256];
	uint8_t preKey_[256];
	HINSTANCE hInstance_;

	// マウス
	ComPtr<IDirectInputDevice8> mouse_;
	DIMOUSESTATE2 mouseState_{};    // 今フレームのマウス入力
	DIMOUSESTATE2 preMouseState_{}; // 前フレーム

	float accumX_ = 0;
	float accumY_ = 0;

	// WindowsAPI
	WinApp* winApp_ = nullptr;
};
