#pragma once
#include "Block.h"
#include "Enemy.h"
#include "Framework.h"
#include "Map.h"
#include "MathUtility.h"
#include "Player.h"
#include "PowerUpItem.h"
#include <memory>

class Game : public Framework {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// シーン種別
	enum class Scene {
		Title = 0,
		Game,
		Result,
	};

	// シーン管理
	void ChangeScene(Scene newScene);
	void StartGameScene(); // ゲームシーン開始（初期化／リセット）

	// マップチップに応じてオブジェクトの配置
	void SpawnObjectsByMapChip(Vector2 leftTop);

	// フレームレートの表示(ImGui)
	void ImGuiFPS();

	// デバッグカメラ(ImGui)
	void ImGuiDebugCamera();

	// 当たり判定(Player, Enemy)
	void CollisionPlayerEnemy();

	// 当たり判定(Enemy, Player, ヒップドロップ)
	void CollisionEnemyPlayerHipDrop();

	// プレイヤーとパワーアップアイテムの当たり判定
	void CollisionPlayerPowerUpItem();

	// パワーアップアイテムの生成
	void CreatePowerUpItem();

	// シェイク初期設定用
	void StartShake(int frames, float magnitude);

	// シェイク関数
	void ShakeCamera();

private:
	// 現在のシーン
	Scene currentScene_ = Scene::Game;

	// model座標
	Transform transform_[5];

	// color
	Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	// プレイヤー
	std::unique_ptr<Player> player_ = std::make_unique<Player>();

	// マップ
	std::unique_ptr<Map> map_ = std::make_unique<Map>();

	// ブロック
	std::vector<std::unique_ptr<Block>> blocks_;

	// マップの左上座標
	Vector2 mapLeftTop_ = {0.0f, 0.0f};

	// 敵
	std::unique_ptr<Enemy> enemy_ = std::make_unique<Enemy>();

	// プレイヤーのパワーアップアイテム
	std::vector<std::unique_ptr<PowerUpItem>> powerUpItems_;

	// パワーアップアイテム生成フレームカウント
	int powerUpItemCreateFrameCount_ = 0;

	// パワーアップアイテムが生成されるまでの最大フレームカウント
	const int kPowerUpItemFrameCountMax = 60;

	// パワーアップアイテムの画面上に存在できる最大数
	const int kPowerUpItemCountMax = 5;

	// 土埃パーティクル プレイヤー用
	std::unique_ptr<Particle> particleDustPlayer_;

	// 土埃パーティクル 敵用
	std::unique_ptr<Particle> particleDustEnemy_;

	// タイトルテキストモデル
	std::unique_ptr<Object3d> titleText_;

	float titleRotateY_ = 0.0f;

	// シェイクフラグ
	bool isShake_ = false;
	int shakeFrames_ = 0;
	int currentFrame_ = 0;
	float magnitude_ = 0.0f;
	Vector3 originalPos_;
};