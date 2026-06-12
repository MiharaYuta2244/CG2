#include "Framework.h"

void Framework::Initialize() {
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// ウィンドウ作成
	winApp_ = std::make_shared<WinApp>();

	// DirectX12 デバイス初期化
	dxCommon_ = std::make_unique<DirectXCommon>();

	// SRVManager
	srvManager_ = std::make_unique<SrvManager>();

#ifdef USE_IMGUI
	// ImGuiManager
	imGuiManager_ = std::make_unique<ImGuiManager>();
#endif

	// テクスチャマネージャー
	textureManager_ = std::make_unique<TextureManager>();

	// Object3dCommon
	object3dCommon_ = std::make_unique<Object3dCommon>();

	// ModelManager
	modelManager_ = std::make_unique<ModelManager>();

	// Sprite共通部
	spriteCommon_ = std::make_unique<SpriteCommon>();

	// 経過時間
	timeManager_ = std::make_unique<TimeManager>();

	// パーティクルコモン
	particleCommon_ = std::make_unique<ParticleCommon>();

	// DirectInput
	input_ = std::make_unique<DirectInput>();

	// GamePad
	gamePad_ = std::make_unique<GamePad>();

	// DebugCamera
	debugCamera_ = std::make_unique<Camera>();

	// SkyboxCommon
	skyboxCommon_ = std::make_unique<SkyboxCommon>();

	// RenderTexture
	renderTexture_ = std::make_unique<RenderTexture>();

	// DirectX12 デバイス初期化
	dxCommon_->Initialize(winApp_);

	// SrvManager
	srvManager_->Initialize(dxCommon_.get());

	// RenderTexture
	renderTexture_->Initialize(dxCommon_.get(), srvManager_.get(), WinApp::kClientWidth, WinApp::kClientHeight);

	// CopyImage
	copyImage_ = std::make_unique<TinyEngine::CopyImage>();
	copyImage_->Initialize(dxCommon_.get(), PostEffectType::DepthOutline);

#ifdef USE_IMGUI
	// ImGuiManager
	imGuiManager_->Initialize(dxCommon_->GetWinApp()->GetHWND(), dxCommon_->GetDevice(), dxCommon_->GetSwapChainDescBufferCount(), dxCommon_->GetRtvFormat(), dxCommon_->GetSrvDescriptorHeap().Get());

	// ImGuiの色変更
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
#endif

	// テクスチャマネージャー
	textureManager_->Initialize(dxCommon_.get(), srvManager_.get());

	// Object3dCommon
	object3dCommon_->Initialize(dxCommon_.get());

	// ModelManager
	modelManager_->Initialize(dxCommon_.get(), textureManager_.get());
	modelManager_->AllModelLoad();

	// Sprite共通部
	spriteCommon_->Initialize(dxCommon_.get());

	// ParticleCommon
	particleCommon_->Initialize(dxCommon_.get());

	// SkyboxCommon
	skyboxCommon_->Initialize(dxCommon_.get());

	// DebugCamera
	debugCamera_->Initialize();
	debugCamera_->SetTranslation({0.0f, 0.0f, -75.0f});
	object3dCommon_->SetDefaultCamera(debugCamera_.get());
	particleCommon_->SetDefaultCamera(debugCamera_.get());

	// コンテキスト構造体
	engineContext_.object3dCommon = object3dCommon_.get();
	engineContext_.spriteCommon = spriteCommon_.get();
	engineContext_.modelManager = modelManager_.get();
	engineContext_.textureManager = textureManager_.get();
	engineContext_.particleCommon = particleCommon_.get();
	engineContext_.srvManager = srvManager_.get();
	engineContext_.skyboxCommon = skyboxCommon_.get();
	engineContext_.copyImage = copyImage_.get();

	// DirectInput
	input_->Initialize(winApp_.get());

	// ログファイルの作成
	Logger::Initialize();
}

void Framework::Update() {
	// シーンに依らず、経過時間と入力は更新しておく
	timeManager_->Update();
	fps_ = 1.0f / timeManager_->GetDeltaTime();
	input_->Update();

#ifdef USE_IMGUI
	// ImGui前処理
	imGuiManager_->BeginFrame();

	// ポストエフェクトのパラメータ調整用
	copyImage_->DrawImGui();
#endif

	// デバッグカメラ更新
	debugCamera_->Update(*input_, *gamePad_);
	gamePad_->Update();
}

void Framework::Finalize() {
#ifdef USE_IMGUI
	imGuiManager_->Finalize();
#endif

	Logger::Finalize();

	CloseHandle(dxCommon_->GetFenceEvent());

	CoUninitialize();
}

void Framework::Run() {
	// ゲームの初期化
	Initialize();

	while (true) {
		// ウィンドウメッセージの処理
		if (!winApp_->ProcessMessage()) {
			break;
		}

		// 毎フレーム更新
		Update();

		// 終了リクエストが来たら抜ける
		if (IsEndRequest()) {
			break;
		}

		// 描画
		Draw();
	}

	// ゲームの終了
	Finalize();
}

void Framework::PreDraw() {
	// 描画開始
	dxCommon_->BeginFrame();
}

void Framework::PostDraw() {
	// SRVヒープをコマンドリストにセットするための前処理
	SRVManagerPreDraw();

	// RenderTextureからSwapChainへコピー
	copyImage_->Draw(dxCommon_.get(), srvManager_.get(), renderTexture_->GetSRVIndexColor(), renderTexture_->GetSRVIndexDepth());

	// ImGuiの内部コマンドを生成する
#ifdef USE_IMGUI
	imGuiManager_->Render(dxCommon_->GetCommandList());
#endif

	// 描画終了
	dxCommon_->EndFrame();
}

void Framework::SRVManagerPreDraw() { srvManager_->PreDraw(); }

void Framework::BeginRender() { renderTexture_->BeginRender(dxCommon_.get()); }

void Framework::EndRender() { renderTexture_->EndRender(dxCommon_.get()); }
