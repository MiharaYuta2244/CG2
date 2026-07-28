#pragma once
#include "BaseScene.h"
#include "Cinematic/CameraDeathZoomController.h"
#include "EasingEditor.h"
#include "GameObjects/Effect/FlashEffect.h"
#include "GameObjects/Effect/LetterBox.h"
#include "GameObjects/Enemy/EnemyBulletManager.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/StageObjects/Stage.h"
#include "DecalManager.h"
#include "Editor/SceneEditor.h"
#include "Particle.h"
#include "Collision/CollisionManager.h"
#include "GameObjects/ControlUI/ControlUI.h"
#include "GameObjects/Enemy/EnemyBombManager.h"
#include <memory>

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
	// グリッチノイズの更新
	void UpdateGlitch(float deltaTime);

	// 敵死亡時パーティクル生成
	void GenerateEnemyDeathEffect(const Vector3& pos);

	void FollowCamera(float deltaTime);

private:
	// プレイヤー
	std::unique_ptr<Player> player_;

	// 敵
	std::unique_ptr<EnemyManager> enemyManager_;

	// 敵の弾を管理するインスタンス
	std::unique_ptr<EnemyBulletManager> enemyBulletManager_;

	// 敵の爆弾を管理するインスタンス
	std::unique_ptr<EnemyBombManager> enemyBombManager_;

	// 死亡パーティクルリスト
	std::list<std::unique_ptr<TinyEngine::Particle>> enemyDeathEffect_;

	// フラッシュエフェクト
	std::unique_ptr<FlashEffect> flashEffect_;

	// レターボックス
	std::unique_ptr<LetterBox> letterBox_;

	// 血痕管理インスタンス
	std::unique_ptr<TinyEngine::DecalManager> decalManager_;

	// プレイヤー死亡時カメラ演出用インスタンス
	std::unique_ptr<CameraDeathZoomController> cameraZoomController_;

	// シーンエディター
	std::unique_ptr<SceneEditor> sceneEditor_;

	// ステージオブジェクト管理インスタンス
	std::unique_ptr<Stage> stage_;

	// 当たり判定管理インスタンス
	std::unique_ptr<CollisionManager> collisionManager_;

	// カメラ関連
	std::unique_ptr<Camera> mainCamera_;  // メインカメラ
	std::unique_ptr<Camera> debugCamera_; // デバッグカメラ
	bool isDebugCameraActive_ = false;    // カメラの切り替えフラグ

	// シーン遷移の要求を1回だけ通るようにするためのフラグ
	bool isTransitionRequested_ = false;

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

	// カメラのピボット位置保持用の変数
	Vector3 currentCameraPivot_ = {0.0f, 0.0f, 0.0f};

	// プレイヤーの向きに応じたカメラのオフセット量
	float offsetDistance_ = 0.0f;

	// カメラのY座標
	float cameraPosY_ = 0.0f;

	// 操作方法UI
	std::unique_ptr<ControlUI> controlUI_;

	// カメラ関連
	float cameraAngle_ = 6.0f; // カメラの傾き
	float tiltSpeed_ = 5.0f; // 傾くスピード
};
