#pragma once
#include "BaseScene.h"
#include "EasingEditor.h"
#include "GameObjects/Effect/FlashEffect.h"
#include "GameObjects/Effect/LetterBox.h"
#include "GameObjects/Enemy/EnemyBulletManager.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Goal/Goal.h"
#include "GameObjects/Ground/Ground.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Player/PlayerHPGauge.h"
#include "GameObjects/Wall/WallManager.h"
#include "Object3d.h"
#include "Particle.h"
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

private:
	// プレイヤー
	std::unique_ptr<Player> player_;

	// 敵
	std::unique_ptr<EnemyManager> enemyManager_;

	// 敵の弾を管理するクラス
	std::unique_ptr<EnemyBulletManager> enemyBulletManager_;

	// 壁の管理クラス
	std::unique_ptr<WallManager> wallManager_;

	// ゴール判定用クラス
	std::unique_ptr<Goal> goal_;

	// 死亡パーティクルリスト
	std::list<std::unique_ptr<TinyEngine::Particle>> enemyDeathParticle_;

	// プレイヤーのHPゲージ
	std::unique_ptr<PlayerHPGauge> playerHPGauge_;

	// オブジェクトのリスト
	std::vector<TinyEngine::Object3d*> objects_;

	// 選択中のオブジェクトポインタ
	TinyEngine::Object3d* selectedObject_ = nullptr;

	// SRTの内扱うパラメータ
	ImGuizmo::OPERATION currentGizmoOperation_ = ImGuizmo::TRANSLATE;

	// 座標系の設定
	ImGuizmo::MODE currentGizmoMode_ = ImGuizmo::LOCAL;

	// イージングエディター
	std::unique_ptr<EasingEditor> easingEditor_;

	// フラッシュエフェクト
	std::unique_ptr<FlashEffect> flashEffect_;

	// レターボックス
	std::unique_ptr<LetterBox> letterBox_;

	// 地面
	std::unique_ptr<Ground> ground_;

	// カメラ関連
	std::unique_ptr<Camera> mainCamera_;  // メインカメラ
	std::unique_ptr<Camera> debugCamera_; // デバッグカメラ
	bool isDebugCameraActive_ = false;    // カメラの切り替えフラグ

	// シーン遷移の要求を1回だけ通るようにするためのフラグ
	bool isTransitionRequested_ = false;
};
