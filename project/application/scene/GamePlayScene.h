#pragma once
#include "BaseScene.h"
#include "Object3d.h"
#include "Particle.h"
#include "Sprite.h"
#include "AudioManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Enemy/EnemyBulletManager.h"
#include "GameObjects/Wall/WallManager.h"
#include "GameObjects/Goal/Goal.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Player/PlayerHPGauge.h"
#include "GameObjects/Effect/FlashEffect.h"
#include "GameObjects/Effect/LetterBox.h"
#include "GameObjects/Ground/Ground.h"
#include "EasingEditor.h"
#include <array>
#include <memory>
#include <vector>

class GamePlayScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* mainCamera, TimeManager* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// 当たり判定
	void CollisionGameObjects();

	// ギズモ用ImGuiの更新
	void UpdateImGui();

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

	// デバッグカメラ
	std::unique_ptr<Camera> debugCamera_;

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
};
