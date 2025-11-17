#pragma once
#include <memory>
#include "Block.h"
#include "D3DResourceLeakChecker.h"
#include "DebugCamera.h"
#include "DeltaTime.h"
#include "DirectInput.h"
#include "DirectXCommon.h"
#include "Enemy.h"
#include "GamePad.h"
#include "ImGuiManager.h"
#include "ImGuizmo.h"
#include "Map.h"
#include "MathUtility.h"
#include "Model.h"
#include "ModelCommon.h"
#include "ModelManager.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Player.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "XAudio.h"
#include "PowerUpItem.h"
#include "ParticleCommon.h"
#include "Particle.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

class Game {
public:
	void Initialize(HINSTANCE hInstance);

	void Update();

	void Draw();

	void Finalize();

	DirectXCommon* GetDxCommon() const { return dxCommon_.get(); }

private:
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

	// モデルを読み込む
	void AllModelLoader();

	// パワーアップアイテムの生成
	void CreatePowerUpItem();

	// 指定された範囲でランダムなfloat値を生成
	float ApplyRandomFloat(float min, float max);

private:
	// リリースリークチェック
	D3DResourceLeakChecker leakCheck_;

	// ウィンドウ作成
	std::shared_ptr<WinApp> winApp_ = std::make_shared<WinApp>();

	// DirectX12 デバイス初期化
	std::shared_ptr<DirectXCommon> dxCommon_ = std::make_unique<DirectXCommon>();

	// SRVManager
	std::unique_ptr<SrvManager> srvManager_ = std::make_unique<SrvManager>();

#ifdef USE_IMGUI
	// ImGuiManager
	std::unique_ptr<ImGuiManager> imGuiManager_ = std::make_unique<ImGuiManager>();
#endif

	// テクスチャマネージャー
	std::unique_ptr<TextureManager> textureManager_ = std::make_unique<TextureManager>();

	// Object3dCommon
	std::unique_ptr<Object3dCommon> object3dCommon_ = std::make_unique<Object3dCommon>();

	// ModelManager
	std::unique_ptr<ModelManager> modelManger_ = std::make_unique<ModelManager>();

	// Object3d
	std::vector<std::unique_ptr<Object3d>> object3ds_;

	// Sprite共通部
	std::unique_ptr<SpriteCommon> spriteCommon_ = std::make_unique<SpriteCommon>();

	// XAudio
	std::unique_ptr<XAudio> audio_ = std::make_unique<XAudio>();

	// DirectInput
	std::unique_ptr<DirectInput> input_ = std::make_unique<DirectInput>();

	// GamePad
	std::unique_ptr<GamePad> gamePad_ = std::make_unique<GamePad>();

	// DebugCamera
	std::unique_ptr<DebugCamera> debugCamera_ = std::make_unique<DebugCamera>();

	// model座標
	Transform transform_[5];

	// color
	Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	// ギズモで動かせるオブジェクトを選ぶ用の変数
	int objIndex_ = 0;

	// プレイヤー
	std::unique_ptr<Player> player_ = std::make_unique<Player>();

	// スプライト
	std::array<std::unique_ptr<Sprite>, 2> sprite_;

	// 経過時間
	std::unique_ptr<DeltaTime> deltaTime_ = std::make_unique<DeltaTime>();

	// マップ
	std::unique_ptr<Map> map_ = std::make_unique<Map>();

	// ブロック
	std::vector<std::unique_ptr<Block>> blocks_;

	// フレームレート
	float fps_ = 0.0f;

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

	// テスト用パーティクルコモン
	std::unique_ptr<ParticleCommon> testParticleCommon_ = std::make_unique<ParticleCommon>();

	// テスト用パーティクル
	std::unique_ptr<Particle> testParticle_ = std::make_unique<Particle>();

	// Test
	std::unique_ptr<Sprite> testSprite_ = std::make_unique<Sprite>();
};
