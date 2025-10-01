#pragma once
#include "D3DResourceLeakChecker.h"
#include "DebugCamera.h"
#include "DirectInput.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "ImGuizmo.h"
#include "MathUtility.h"
#include "Model.h"
#include "ModelCommon.h"
#include "ModelManager.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "XAudio.h"
#include <memory>
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
	// リリースリークチェック
	D3DResourceLeakChecker leakCheck_;

	// ウィンドウ作成
	std::shared_ptr<WinApp> winApp_ = std::make_shared<WinApp>();

	// DirectX12 デバイス初期化
	std::shared_ptr<DirectXCommon> dxCommon_ = std::make_unique<DirectXCommon>();

	// ImGuiManager
	std::unique_ptr<ImGuiManager> imGuiManager_ = std::make_unique<ImGuiManager>();

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

	// Sprite
	std::vector<std::unique_ptr<Sprite>> sprites_;

	// XAudio
	std::unique_ptr<XAudio> audio_ = std::make_unique<XAudio>();

	// DirectInput
	std::unique_ptr<DirectInput> input_ = std::make_unique<DirectInput>();

	// DebugCamera
	std::unique_ptr<DebugCamera> debugCamera_ = std::make_unique<DebugCamera>();

	// model座標
	Transform transform_[5];

	// color
	Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	// ギズモで動かせるオブジェクトを選ぶ用の変数
	int objIndex_ = 0;
};
