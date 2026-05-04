#pragma once
#include "D3DResourceLeakChecker.h"
#include "DirectXCommon.h"
#include "EngineContext.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"
#include "SpriteCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "TimeManager.h"
#include "DebugCamera.h"
#include "DirectInput.h"
#include "GamePad.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

/// <summary>
/// フレームワーククラス
/// </summary>
class Framework {
public:
	virtual ~Framework() = default;

	virtual void Initialize();

	virtual void Finalize();

	virtual void Update();

	virtual void Draw()=0;

	// 実行
	void Run();

	// 描画前処理
	void PreDraw();

	// 描画後処理
	void PostDraw();

	// 終了チェック
	virtual bool IsEndRequest() { return endRequest_; }

	// DXCommonのGetter
	DirectXCommon* GetDxCommon() const { return dxCommon_.get(); }

	// コンテキスト構造体のGetter
	EngineContext& GetEngineContext() { return engineContext_; }

	// キーボード入力のGetter
	DirectInput* GetKeyboard() { return input_.get(); }

	// ゲームパッド入力のGetter
	GamePad* GetGamePad() { return gamePad_.get(); }

	// タイムマネージャーのGetter
	TimeManager* GetTimeManager() { return timeManager_.get(); }

	// デバッグカメラのGetter
	Camera* GetDebugCamera() { return debugCamera_.get(); }

	// フレームレートのGetter
	float GetFPS() { return fps_; }

private:
	bool endRequest_; // 終了フラグ

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
	std::unique_ptr<ModelManager> modelManager_ = std::make_unique<ModelManager>();

	// Sprite共通部
	std::unique_ptr<SpriteCommon> spriteCommon_ = std::make_unique<SpriteCommon>();

	// 経過時間
	std::unique_ptr<TimeManager> timeManager_ = std::make_unique<TimeManager>();

	// パーティクルコモン
	std::unique_ptr<ParticleCommon> particleCommon_ = std::make_unique<ParticleCommon>();

	// DirectInput
	std::unique_ptr<DirectInput> input_ = std::make_unique<DirectInput>();

	// GamePad
	std::unique_ptr<GamePad> gamePad_ = std::make_unique<GamePad>();

	// DebugCamera
	std::unique_ptr<Camera> debugCamera_ = std::make_unique<Camera>();

	// コンテキスト構造体
	EngineContext engineContext_;

	// フレームレート
	float fps_ = 0.0f;
};
