#include "Game.h"

void Game::Initialize(HINSTANCE hInstance) {
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// DirectX12 デバイス初期化
	dxCommon_->Initialize(winApp_);

	// SrvManager
	srvManager_->Initialize(dxCommon_.get());

	// ImGuiManager
	imGuiManager_->Initialize(dxCommon_->GetWinApp()->GetHWND(), dxCommon_->GetDevice(), dxCommon_->GetSwapChainDescBufferCount(), dxCommon_->GetRtvFormat(), dxCommon_->GetSrvDescriptorHeap().Get());

	// ImGuiの色変更
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // ダークグレー
	style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);     // 明るいグレー

	// テクスチャマネージャー
	textureManager_->Initialize(dxCommon_.get(), srvManager_.get());

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
	modelManger_->LoadModel("Box.obj");

	// modelのポインタを受け取る
	// object3ds_[0]->SetModel("fence.obj");
	// object3ds_[1]->SetModel("axis.obj");
	// object3ds_[2]->SetModel("SkySphere.obj");
	// object3ds_[3]->SetModel("sphere.obj");
	// object3ds_[4]->SetModel("Field.obj");

	// Sprite共通部
	spriteCommon_->Initialize(dxCommon_.get());

	// XAudio
	audio_->Initialize();
	audio_->SoundsAllLoad();

	// DirectInput
	input_->Initialize(winApp_.get());

	// DebugCamera
	debugCamera_->Initialize();
	debugCamera_->SetTranslation({19.45f, 10.5f, -50.0f});
	object3dCommon_->SetDefaultCamera(debugCamera_.get());
	//particleCommon_->SetDefaultCamera(debugCamera_.get());

	// プレイヤー
	player_->Initialize(object3dCommon_.get(), textureManager_.get(), modelManger_.get(), input_.get(), gamePad_.get());
	player_->SetModel("sphere.obj");

	// マップ
	//map_->Initialize();

	// 敵
	//enemy_->Initialize(object3dCommon_.get(), textureManager_.get(), modelManger_.get());
	//enemy_->SetModel("Box.obj");

	// オブジェクトの配置
	//SpawnObjectsByMapChip(mapLeftTop_);
}

void Game::Update() {
	// 経過時間
	deltaTime_->Update();

	// フレームレート
	fps_ = 1.0f / deltaTime_->GetDeltaTime();

	// 入力処理更新
	input_->Update();

	// ImGui前処理
	imGuiManager_->BeginFrame();

	// プレイヤーのimGui
	player_->UpdateImGui();

	// 敵のImGui
	//enemy_->UpdateImGui();

	// フレームレート表示(ImGui)
	ImGuiFPS();

	// デバッグカメラ(ImGui)
	ImGuiDebugCamera();

	// ImGuiウィンドウ位置、サイズ固定
	// ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	// ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always);

	// ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); // 完全に透明
	// ImGui::Begin("Gizmo", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);
	// ImGuizmo::BeginFrame();

	// ImGuizmoの設定
	// ImGuizmo::SetOrthographic(false);
	// ImGuizmo::Enable(true);
	// ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

	// ImGuiIO& io = ImGui::GetIO();
	// float windowWidth = (float)ImGui::GetWindowWidth();
	// float windowHeight = (float)ImGui::GetWindowHeight();

	// ウィンドウサイズを取得してImGuizmoに渡す
	// ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

	// デバッグカメラ更新
	debugCamera_->Update(*input_, *gamePad_);

	// ImGuizmo::Manipulate(
	//     *(debugCamera_->GetViewMatrix()).m, *(object3ds_[objIndex_]->GetProjectionMatrix()).m, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, *(object3ds_[objIndex_]->GetWorldMatrix()).m);

	// ImGui::End();
	// ImGui::PopStyleColor();

	// Particle更新
	//particle_->Update();

	// プレイヤー更新
	player_->Update(deltaTime_->GetDeltaTime());

	// ブロック更新
	//for (auto& block : blocks_) {
	//	block->Update();
	//}

	// 敵更新
	//enemy_->Update(deltaTime_->GetDeltaTime());
}

void Game::Draw() {
	// 描画開始
	dxCommon_->BeginFrame();

	// プレイヤー描画
	player_->Draw();

	// ブロック描画
	// for (auto& block : blocks_) {
	//	block->Draw();
	//}

	// 敵描画
	// enemy_->Draw();

	// Particle描画
	//particle_->Draw();

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

void Game::SpawnObjectsByMapChip(Vector2 leftTop) {
	for (int y = 0; y < map_->GetRowCount(); ++y) {
		for (int x = 0; x < map_->GetColumnCount(); ++x) {
			int tile = map_->GetMapData(y, x);

			// タイルごとの描画処理
			if (static_cast<TileType>(tile) == TileType::BLOCK) {
				// ブロックの初期化
				auto block = std::make_unique<Block>();
				block->Initialize(object3dCommon_.get(), textureManager_.get(), modelManger_.get());
				block->Spawn({static_cast<float>(x), static_cast<float>(y), 0.0f});
				block->SetModel("Box.obj");
				block->GetObject3d()->SetColor({0.8f, 0.8f, 0.0f, 1.0f});
				blocks_.push_back(std::move(block));
			}
		}
	}
}

void Game::ImGuiFPS() {
	ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("FPS: %.1f", fps_);
	ImGui::Text("Frame Time: %.3f ms", deltaTime_->GetDeltaTime() * 1000.0f);
	ImGui::End();
}

void Game::ImGuiDebugCamera() {
	ImGui::Begin("DebugCamera");
	ImGui::DragFloat3("Pos", &debugCamera_->GetTranslation().x, 0.01f);
	ImGui::End();
}
