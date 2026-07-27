#pragma once
#include "Cinematic/CameraDeathZoomController.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/IGameObject.h"
#include "ImGuizmo.h"
#include "SceneContext.h"
#include "EditorCommand.h"
#include <vector>
#include <deque>

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

	// 対象オブジェクトが現在もobjects_内に存在するかを確認する
	bool IsObjectAlive(IGameObject* obj) const;

	// コマンドをUndoスタックに積む
	void PushCommand(std::unique_ptr<IEditorCommand> command);

	// 直前の操作を取り消して元に戻す
	void Undo();

	// 取り消した操作をもう一度やり直す
	void Redo();

	// Ctrl+Z / Ctrl+Yの入力を処理する
	void HandleUndoRedoInput(const SceneContext& ctx);

private:
	// オブジェクトのリスト
	std::vector<IGameObject*> objects_;

	// 選択中のオブジェクトポインタ
	IGameObject* selectedGameObject_ = nullptr;

	// SRTの内扱うパラメータ
	ImGuizmo::OPERATION currentGizmoOperation_ = ImGuizmo::TRANSLATE;

	// 座標系の設定
	ImGuizmo::MODE currentGizmoMode_ = ImGuizmo::LOCAL;

	// Undo/Redo用ステート
	std::deque<std::unique_ptr<IEditorCommand>> undoStack_;
	std::deque<std::unique_ptr<IEditorCommand>> redoStack_;
	static constexpr size_t kMaxHistorySize = 100;

	// ImGuizmo操作中フラグと操作開始時のスナップショット
	bool wasGizmoUsing_ = false;
	TransformSnapshot gizmoBeginSnapshot_{};

	// 手動SRT編集用の操作開始スナップショット
	TransformSnapshot dragBeginSnapshot_{};
};