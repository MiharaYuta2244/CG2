#pragma once

#include <Windows.h>

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include <cstdint>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

class WinApp {
public:
	WinApp();
	~WinApp();

	// getter
	HINSTANCE GetHInstance() const { return wc_.hInstance; }
	HWND GetHWND() const { return hwnd_; }
	bool ProcessMessage();

public:
	// ウィンドウサイズ
	static inline const int32_t kClientWidth = 1280;
	static inline const int32_t kClientHeight = 720;

private:
	// WindowsAPIに渡す関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	HWND hwnd_ = nullptr;
	WNDCLASS wc_{};
};
