#pragma once
#include "Decalmanager.h"

/// <summary>
/// 操作方法のUI管理インスタンス
/// </summary>
class ControlUI {
public:
	// 初期化関数
	void Initialize(EngineContext* ctx, TinyEngine::DecalManager* uiDecalManager);

	// 更新処理
	void Update();

	// デカールの追加
	void AddAttackUIDecal(Vector3 scale);
	void AddHoldUIDecal(Vector3 scale);

	// ImGui
	void DrawImGui();

private:
	// デカール管理インスタンス
	TinyEngine::DecalManager* uiDecalManager_;

	// 座標
	Vector3 attackUIPos_;
	Vector3 holdUIPos_;

	// デカールデータへの参照ポインタ
	TinyEngine::DecalManager::DecalData* attackDecal_ = nullptr;
	TinyEngine::DecalManager::DecalData* holdDecal_ = nullptr;
};
