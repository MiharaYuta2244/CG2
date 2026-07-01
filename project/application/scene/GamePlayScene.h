#pragma once
#include "BaseScene.h"
#include "Cinematic/CameraDeathZoomController.h"
#include "EasingEditor.h"
#include "GameObjects/Effect/FlashEffect.h"
#include "GameObjects/Effect/LetterBox.h"
#include "GameObjects/Enemy/EnemyBulletManager.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/GameUI/Controls.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/StageObjects/Door/DoorManager.h"
#include "GameObjects/StageObjects/Goal/Goal.h"
#include "GameObjects/StageObjects/Ground/Ground.h"
#include "GameObjects/StageObjects/Wall/WallManager.h"
#include "Object3d.h"
#include "Particle.h"
#include "Skybox.h"
#include <memory>
#include <vector>

/// <summary>
/// ゲームシーン
/// </summary>
class GamePlayScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// 当たり判定
	void CollisionGameObjects();

	// ギズモ用ImGuiの更新
	void UpdateImGui();

	// デバッグ用のImGui更新
	void UpdateDebugImGui();

	// グリッチノイズの更新
	void UpdateGlitch(float deltaTime);

	// 敵死亡時パーティクル生成
	void GenerateEnemyDeathParticle(const Vector3& pos);

private:
	// プレイヤー
	std::unique_ptr<Player> player_;

	// 敵
	std::unique_ptr<EnemyManager> enemyManager_;

	// 敵の弾を管理するクラス
	std::unique_ptr<EnemyBulletManager> enemyBulletManager_;

	// 壁の管理クラス
	std::unique_ptr<WallManager> wallManager_;

	// ドアの管理クラス
	std::unique_ptr<DoorManager> doorManager_;

	// ゴール判定用クラス
	std::unique_ptr<Goal> goal_;

	// 死亡パーティクルリスト
	std::list<std::unique_ptr<TinyEngine::Particle>> enemyDeathParticle_;

	// オブジェクトのリスト
	std::vector<IGameObject*> objects_;

	// 選択中のオブジェクトポインタ
	IGameObject* selectedGameObject_ = nullptr;

	// SRTの内扱うパラメータ
	ImGuizmo::OPERATION currentGizmoOperation_ = ImGuizmo::TRANSLATE;

	// 座標系の設定
	ImGuizmo::MODE currentGizmoMode_ = ImGuizmo::LOCAL;

	// フラッシュエフェクト
	std::unique_ptr<FlashEffect> flashEffect_;

	// レターボックス
	std::unique_ptr<LetterBox> letterBox_;

	// 地面
	std::unique_ptr<Ground> ground_;

	// スカイボックス
	std::unique_ptr<TinyEngine::Skybox> skybox_;

	// カメラ関連
	std::unique_ptr<Camera> mainCamera_;  // メインカメラ
	std::unique_ptr<Camera> debugCamera_; // デバッグカメラ
	bool isDebugCameraActive_ = false;    // カメラの切り替えフラグ

	// シーン遷移の要求を1回だけ通るようにするためのフラグ
	bool isTransitionRequested_ = false;

	// 操作方法UI
	std::unique_ptr<Controls> controls_;

	// 経過時間
	float elapsedTime_ = 0.0f;

	// グリッチノイズ用変数
	float glitchTimer_ = 0.0f;          // 経過時間
	const float kGlitchDuration = 0.1f; // グリッチノイズをかける時間

	// カメラのシェイク量
	float shakePower_ = 2.0f;

	// プレイヤー死亡時演出用RadialBlurのNumSamplesAnim
	AnimationBundle<float> numSamplesAnim_;
	bool isDeathAnimStarted_ = false; // アニメーションの開始フラグ

	// プレイヤー死亡時カメラ演出用インスタンス
	std::unique_ptr<CameraDeathZoomController> cameraZoomController_;

	// カメラのピボット位置保持用の変数
	Vector3 currentCameraPivot_ = {0.0f, 0.0f, 0.0f};

	// プレイヤーの向きに応じたカメラのオフセット量
	float offsetDistance_ = 0.0f;

	float cameraPosY_ = 0.0f;
};
