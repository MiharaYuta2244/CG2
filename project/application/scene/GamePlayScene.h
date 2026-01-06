#pragma once
#include "Apple.h"
#include "BaseScene.h"
#include "Block.h"
#include "BothCurtain.h"
#include "Enemy.h"
#include "Grape.h"
#include "Map.h"
#include "Object3d.h"
#include "Orange.h"
#include "Particle.h"
#include "Player.h"
#include "PowerUpItem.h"
#include "Sprite.h"
#include "Cloud.h"
#include <array>
#include <memory>
#include <vector>

class GamePlayScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// ゲームシーン初期化
	void StartGameScene();

	// マップチップに応じてオブジェクトの配置
	void SpawnObjectsByMapChip(Vector2 leftTop);

	// 当たり判定(Player, Enemy)
	void CollisionPlayerEnemy();

	// 当たり判定(Enemy, Player, ヒップドロップ)
	void CollisionEnemyPlayerHipDrop();

	// プレイヤーとパワーアップアイテムの当たり判定
	void CollisionPlayerPowerUpItem();

	// プレイヤーとフルーツの当たり判定
	void CollisionPlayerFruits();

	// パワーアップアイテムの生成
	void CreatePowerUpItem();

	// ブドウの生成
	void CreateGrape();

	// リンゴの生成
	void CreateApple();

	// オレンジの生成
	void CreateOrange();

	// シェイク初期設定用
	void StartShake(int frames, float magnitude);

	// シェイク関数
	void ShakeCamera();

	// リザルト文字列を保存
	void SaveResultStatus(const std::string& status);

#ifdef USE_IMGUI
	// フレームレートの表示
	void ImGuiFPS();

	// デバッグカメラ
	void ImGuiDebugCamera();
#endif

private:
	struct FruitGenerator {
		float generateTimer;
		const float kGenerateTimer;
		int kCount;
	};

private:
	// プレイヤー
	std::unique_ptr<Player> player_;

	// マップ
	std::unique_ptr<Map> map_;

	// ブロック
	std::vector<std::unique_ptr<Block>> blocks_;

	// マップの左上座標
	Vector2 mapLeftTop_ = {0.0f, 0.0f};

	// 敵
	std::unique_ptr<Enemy> enemy_;

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

	// 木のモデル
	std::array<std::unique_ptr<Object3d>, 10> treeModels_;

	// 地形モデル
	std::unique_ptr<Object3d> terrainModel_;

	// 画面両端の幕
	std::unique_ptr<BothCurtain> rightCurtain_;
	std::unique_ptr<BothCurtain> leftCurtain_;

	// フィールドモデル
	std::unique_ptr<Object3d> fieldModel_;

	// 雲
	std::array<std::unique_ptr<Cloud>,10> clouds_;

	// ヒップドロップゲージ
	std::unique_ptr<PlayerGauge> hipDropGauge_;

	// ブドウ
	std::vector<std::unique_ptr<Grape>> grapes_;

	// リンゴ
	std::vector<std::unique_ptr<Apple>> apples_;

	// オレンジ
	std::vector<std::unique_ptr<Orange>> oranges_;

	// ブドウの生成構造体
	FruitGenerator grapeGenarator_ = {
	    .generateTimer = 0.0f,
	    .kGenerateTimer = 1.0f,
	    .kCount = 5,
	};

	// リンゴの生成構造体
	FruitGenerator appleGenarator_ = {
	    .generateTimer = 0.0f,
	    .kGenerateTimer = 1.0f,
	    .kCount = 5,
	};

	// オレンジの生成構造体
	FruitGenerator orangeGenarator_ = {
	    .generateTimer = 0.0f,
	    .kGenerateTimer = 1.0f,
	    .kCount = 5,
	};

	// シェイクフラグ
	bool isShake_ = false;
	int shakeFrames_ = 0;
	int currentFrame_ = 0;
	float magnitude_ = 0.0f;
	Vector3 originalPos_;

	// 木のアニメーション用
	float t_ = 0.0f;
	const float kTimer = 2.0f;
	float timer_ = 0.0f;
};
