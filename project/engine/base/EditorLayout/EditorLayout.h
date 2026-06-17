#pragma once
#ifdef _DEBUG
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "imgui.h"

/// <summary>
/// Unityライクなエディターレイアウト管理クラス
/// </summary>
class EditorLayout {
public:
	void Initialize(SrvManager* srv, DirectXCommon* dx);
	void BeginEditorLayout(); // フレーム先頭で呼ぶ
	void EndEditorLayout();   // フレーム末尾で呼ぶ

	// GameビューにRenderTextureを表示
	void DrawGameView(uint32_t colorSrvIndex);

	// 各パネル
	void DrawMainMenuBar();
	void DrawHierarchyPanel();
	void DrawInspectorPanel();
	void DrawProjectPanel();

	bool IsGamePlaying() const { return isPlaying_; }

private:
	SrvManager* srv_ = nullptr;
	DirectXCommon* dx_ = nullptr;

	bool isPlaying_ = false;

	// 選択中オブジェクトID
	int selectedObjectId_ = -1;

	// パネルサイズ定数
	static constexpr float kMenuBarHeight = 20.0f;
	static constexpr float kHierarchyWidth = 250.0f;
	static constexpr float kInspectorWidth = 300.0f;
	static constexpr float kProjectHeight = 200.0f;
};
#endif