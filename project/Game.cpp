#include "Game.h"

void Game::Initialize(HINSTANCE hInstance) {
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// DirectX12 デバイス初期化
	dxCommon_->Initialize(winApp_);

	// ImGuiManager
	imGuiManager_->Initialize(dxCommon_->GetWinApp()->GetHWND(), dxCommon_->GetDevice(), dxCommon_->GetSwapChainDescBufferCount(), dxCommon_->GetRtvFormat(), dxCommon_->GetSrvDescriptorHeap().Get());

	// テクスチャマネージャー
	textureManager_->Initialize(dxCommon_.get());

	// Object3dCommon
	object3dCommon_->Initialize(dxCommon_.get());

	// ModelManager
	modelManger_->Initialize(dxCommon_.get(), textureManager_.get());

	// ParticleCommon
	//particleCommon_->Initialize(dxCommon_.get());

	// Particle
	//particle_->Initialize(particleCommon_.get(), textureManager_.get(), modelManger_.get());

	// .objファイルからモデルを読み込む
	modelManger_->LoadModel("fence.obj");
	modelManger_->LoadModel("plane.obj");
	modelManger_->LoadModel("axis.obj");
	modelManger_->LoadModel("SkySphere.obj");
	modelManger_->LoadModel("skydome.obj");
	modelManger_->LoadModel("Field.obj");
	modelManger_->LoadModel("sphere.obj");

	// modelのポインタを受け取る
	//object3ds_[0]->SetModel("fence.obj");
	//object3ds_[1]->SetModel("skydome.obj");
	//object3ds_[2]->SetModel("SkySphere.obj");
	//object3ds_[3]->SetModel("sphere.obj");
	//object3ds_[4]->SetModel("Field.obj");

	// Sprite共通部
	spriteCommon_->Initialize(dxCommon_.get());

	// XAudio
	audio_->Initialize();
	audio_->SoundsAllLoad();

	// DirectInput
	input_->Initialize(winApp_.get());

	// DebugCamera
	debugCamera_->Initialize();

	// プレイヤー
	player_->Initialize(object3dCommon_.get(), textureManager_.get(), modelManger_.get(), input_.get(), gamePad_.get());
	player_->SetModel("fence.obj");
}

void Game::Update() {
	// 経過時間
	deltaTime_->Update();

	// 入力処理更新
	input_->Update();

	// ImGui前処理
	imGuiManager_->BeginFrame();

	// プレイヤーのimGui
	player_->UpdateImGui();

	// ImGuiウィンドウ位置、サイズ固定
	//ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	//ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always);

	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); // 完全に透明
	//ImGui::Begin("Gizmo", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);
	//ImGuizmo::BeginFrame();

	// ImGuizmoの設定
	//ImGuizmo::SetOrthographic(false);
	//ImGuizmo::Enable(true);
	//ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

	//ImGuiIO& io = ImGui::GetIO();
	//float windowWidth = (float)ImGui::GetWindowWidth();
	//float windowHeight = (float)ImGui::GetWindowHeight();

	// ウィンドウサイズを取得してImGuizmoに渡す
	//ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

	// デバッグカメラ更新
	debugCamera_->Update(*input_, *gamePad_);
	player_->GetObject3d()->SetViewMatrix(debugCamera_->GetViewMatrix());

	//ImGuizmo::Manipulate(
	//    *(debugCamera_->GetViewMatrix()).m, *(object3ds_[objIndex_]->GetProjectionMatrix()).m, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, *(object3ds_[objIndex_]->GetWorldMatrix()).m);

	//ImGui::End();
	//ImGui::PopStyleColor();

	// Particle更新
	//particle_->Update();

	// プレイヤー更新
	player_->Update(deltaTime_->GetDeltaTime());
}

void Game::Draw() {
	// 描画開始
	dxCommon_->BeginFrame();

	// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spriteCommon_->DrawSettingCommon();

	// 3Dオブジェクト描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	object3dCommon_->DrawSettingCommon();

	// Particle描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	//particleCommon_->DrawSettingCommon();

	// Particle描画
	//particle_->Draw();

	// プレイヤー描画
	player_->Draw();

	// ImGuiの内部コマンドを生成する
	imGuiManager_->Render(dxCommon_->GetCommandList());

	// 描画終了
	dxCommon_->EndFrame();
}

void Game::Finalize() {
	imGuiManager_->Finalize();

	CloseHandle(dxCommon_->GetFenceEvent());

	CoUninitialize();
}