#pragma once
#include "Cinematic/CameraDeathZoomController.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/IGameObject.h"
#include "ImGuizmo.h"
#include "SceneContext.h"
#include <imgui.h>
#include <vector>

/// <summary>
/// ゲームシーンのデバッグ・エディタ機能を担うクラス
/// </summary>
class SceneEditor {
public:
	void Initialize();

	// 毎フレームの更新処理
	void Update(
	    const SceneContext& ctx, const std::vector<IGameObject*>& objects, Player* player, EnemyManager* enemyManager, CameraDeathZoomController* cameraZoomController, float& cameraPosY,
	    bool& isDebugCameraActive, Vector3& currentCameraPivot);

private:
	// ImGuiの統合更新
	void UpdateImGui(const SceneContext& ctx, Player* player, float& cameraPosY);

	// マウスの画面座標をRayに変換して判定を取る
	void UpdatePicking(const SceneContext& ctx);

	// 入力系デバッグ処理
	void DebugInput(const SceneContext& ctx, Player* player, EnemyManager* enemyManager, CameraDeathZoomController* cameraZoomController, bool& isDebugCameraActive, Vector3& currentCameraPivot);

private:
	// オブジェクトのリスト（毎フレームGamePlaySceneから受け取る）
	std::vector<IGameObject*> objects_;

	// 選択中のオブジェクトポインタ
	IGameObject* selectedGameObject_ = nullptr;

	// SRTの内扱うパラメータ
	ImGuizmo::OPERATION currentGizmoOperation_ = ImGuizmo::TRANSLATE;

	// 座標系の設定
	ImGuizmo::MODE currentGizmoMode_ = ImGuizmo::LOCAL;
};