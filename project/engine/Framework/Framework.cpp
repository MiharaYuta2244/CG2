#include "Framework.h"

void Framework::Initialize() {
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// DirectX12 デバイス初期化
	dxCommon_->Initialize(winApp_);

	// SrvManager
	srvManager_->Initialize(dxCommon_.get());

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
	modelManger_->Initialize(dxCommon_.get(), textureManager_.get());

	// Sprite共通部
	spriteCommon_->Initialize(dxCommon_.get());

	// ParticleCommon
	particleCommon_->Initialize(dxCommon_.get());

	// DebugCamera
	debugCamera_->Initialize();
	debugCamera_->SetTranslation({19.45f, 10.5f, -75.0f});
	object3dCommon_->SetDefaultCamera(debugCamera_.get());
	particleCommon_->SetDefaultCamera(debugCamera_.get());

	// コンテキスト構造体
	engineContext_.object3dCommon = object3dCommon_.get();
	engineContext_.spriteCommon = spriteCommon_.get();
	engineContext_.modelManager = modelManger_.get();
	engineContext_.textureManager = textureManager_.get();
	engineContext_.particleCommon = particleCommon_.get();
	engineContext_.srvManager = srvManager_.get();

	// .objファイルからモデルを読み込む
	AllModelLoader();

	// XAudio
	audio_->Initialize();
	audio_->SoundsAllLoad();
	// audio_->SoundPlayWave();

	// DirectInput
	input_->Initialize(winApp_.get());
}

void Framework::Update() {
	// シーンに依らず、経過時間と入力は更新しておく
	timeManager_->Update();
	fps_ = 1.0f / timeManager_->GetDeltaTime();
	input_->Update();

#ifdef USE_IMGUI
	// ImGui前処理
	imGuiManager_->BeginFrame();
#endif

	// デバッグカメラ更新
	debugCamera_->Update(*input_, *gamePad_);
	gamePad_->Update();
}

void Framework::Finalize() {
#ifdef USE_IMGUI
	imGuiManager_->Finalize();
#endif

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

	srvManager_->PreDraw();
}

void Framework::PostDraw() {
	// ImGuiの内部コマンドを生成する
#ifdef USE_IMGUI
	imGuiManager_->Render(dxCommon_->GetCommandList());
#endif

	// 描画終了
	dxCommon_->EndFrame();
}

void Framework::AllModelLoader() {
	modelManger_->LoadModel("fence.obj");
	modelManger_->LoadModel("plane.obj");
	modelManger_->LoadModel("axis.obj");
	modelManger_->LoadModel("SkySphere.obj");
	modelManger_->LoadModel("skydome.obj");
	modelManger_->LoadModel("Field.obj");
	modelManger_->LoadModel("sphere.obj");
	modelManger_->LoadModel("Box.obj");
	modelManger_->LoadModel("Boss.obj");
	modelManger_->LoadModel("Hiyoko.obj");
	modelManger_->LoadModel("HiyokoGlass.obj");
	modelManger_->LoadModel("HiyokoStudent.obj");
	modelManger_->LoadModel("HiyokoAfro.obj");
	modelManger_->LoadModel("Title.obj");
	modelManger_->LoadModel("HiyokoPropeller.obj");
}