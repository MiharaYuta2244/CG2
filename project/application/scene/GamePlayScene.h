#pragma once
#include "Fruit/Apple/Apple.h"
#include "BaseScene.h"
#include "Block/Block.h"
#include "BothCurtain/BothCurtain.h"
#include "Enemy/Enemy.h"
#include "Fruit/Grape/Grape.h"
#include "Map/Map.h"
#include "Object3d.h"
#include "Fruit/Orange/Orange.h"
#include "Particle.h"
#include "Player/Player.h"
#include "PowerUpItem/PowerUpItem.h"
#include "Sprite.h"
#include "Cloud/Cloud.h"
#include "XAudio.h"
#include "GunTurret/GunTurret.h"
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

	// プレイヤーとフルーツの当たり判定
	void CollisionPlayerFruits();

	// ブドウの生成
	void CreateGrape();

	// リンゴの生成
	void CreateApple();

	// オレンジの生成
	void CreateOrange();

	// 目標フルーツの生成
	void GenerateTargetFruits();

	// 目標フルーツの描画
	void DrawTargetFruits();

	// 目標フルーツの判定と更新
	void UpdateTargetFruits();

	// シェイク初期設定用
	void StartShake(int frames, float magnitude);

	// シェイク関数
	void ShakeCamera();

	// リザルト文字列を保存
	void SaveResultStatus(const std::string& status);

	// ゲームの終了判定
	void EndGameCheck();

	// 木のモデルアニメーション
	void TreeAnimation();

	// ヒップドロップダメージ表示更新
	void UpdateHipDropDamageDisplay();

#ifdef USE_IMGUI
	// フレームレートの表示
	void ImGuiFPS();

	// デバッグカメラ
	void ImGuiDebugCamera();

	// ImGuiデバッグ表示
	void ImGuiUpdate();
#endif

private:
	struct FruitGenerator {
		float generateTimer;
		const float kGenerateTimer;
		int kCount;
	};

	// 目標フルーツ構造体
	struct TargetFruit {
		std::string fruitType; // "Grape", "Apple", "Orange"
		bool isCollected = false;
		std::unique_ptr<Sprite> sprite; // 個別のスプライト
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

	// 土埃パーティクル プレイヤー用
	std::unique_ptr<Particle> particleDustPlayer_;

	// 土埃パーティクル 敵用
	std::unique_ptr<Particle> particleDustEnemy_;

	// 木のモデル
	std::array<std::unique_ptr<Object3d>, 10> treeModels_;

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
	FruitGenerator grapeGenerator_ = {
	    .generateTimer = 0.0f,
	    .kGenerateTimer = 1.0f,
	    .kCount = 5,
	};

	// リンゴの生成構造体
	FruitGenerator appleGenerator_ = {
	    .generateTimer = 0.0f,
	    .kGenerateTimer = 1.0f,
	    .kCount = 5,
	};

	// オレンジの生成構造体
	FruitGenerator orangeGenerator_ = {
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

	// フルーツの画像集
	std::array<std::unique_ptr<Sprite>,3> fruitSprites_;

	// 目標フルーツ
	std::unordered_map<std::string, Sprite*> fruitMap_;

	// 目標配列（目標フルーツの可変長配列）
	std::vector<TargetFruit> fruitOrder_;

	// 初期フルーツ数
	int initialFruitCount_ = 3;

	// 収集済みフルーツ数
	int collectedFruitCount_ = 0;

	// 目標フルーツの背景
	std::unique_ptr<Sprite> targetFruitBG_;

	// 目標フルーツの最大数
	const int kMaxFruitCount_= 10;

	// 数字画像集
	std::array<std::unique_ptr<Sprite>,10> numberSprites_;

	// ヒップドロップダメージ表示用スプライト
	std::array<std::unique_ptr<Sprite>, 2> hipDropDamageSprites_;

	// プレイヤーのヒップドロップパワースプライト
	std::unique_ptr<Sprite> hipDropPowerSprite_;

	// サウンド管理クラス
	std::unique_ptr<XAudio> audio_;

	// 砲台
	std::unique_ptr<GunTurret> gunTurret_;
};
