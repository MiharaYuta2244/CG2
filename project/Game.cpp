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

	// Object3d
	for (uint32_t i = 0; i < 5; ++i) {
		auto object3d = std::make_unique<Object3d>();
		object3d->Initialize(object3dCommon_.get(), textureManager_.get(), modelManger_.get());
		object3ds_.push_back(std::move(object3d));
	}

	// .objファイルからモデルを読み込む
	modelManger_->LoadModel("fence.obj");
	modelManger_->LoadModel("plane.obj");
	modelManger_->LoadModel("axis.obj");
	modelManger_->LoadModel("SkySphere.obj");
	modelManger_->LoadModel("skydome.obj");
	modelManger_->LoadModel("Field.obj");
	modelManger_->LoadModel("sphere.obj");

	// modelのポインタを受け取る
	object3ds_[0]->SetModel("fence.obj");
	object3ds_[1]->SetModel("skydome.obj");
	object3ds_[2]->SetModel("SkySphere.obj");
	object3ds_[3]->SetModel("sphere.obj");
	object3ds_[4]->SetModel("Field.obj");

	object3ds_[1]->SetEnableFoging(false);
	object3ds_[2]->SetEnableFoging(false);
	object3ds_[3]->SetEnableFoging(false);
	object3ds_[4]->SetEnableFoging(false);

	object3ds_[1]->SetColor({1.0f, 1.0f, 1.0f, 0.5f});
	object3ds_[3]->SetColor({1.0f, 1.0f, 1.0f, 0.5f});

	// Sprite共通部
	spriteCommon_->Initialize(dxCommon_.get());

	// Sprite
	for (uint32_t i = 0; i < 5; ++i) {
		auto sprite = std::make_unique<Sprite>();
		sprite->Initialize(spriteCommon_.get(), textureManager_.get(), "resources/uvChecker.png");
		sprites_.push_back(std::move(sprite));
	}

	// XAudio
	audio_->Initialize();
	audio_->SoundsAllLoad();

	// DirectInput
	input_->Initialize(hInstance, dxCommon_->GetWinApp()->GetHWND());

	// DebugCamera
	debugCamera_->Initialize();
}

void Game::Update() {
	// 入力処理更新
	input_->Update();

	imGuiManager_->BeginFrame();

	// ImGuiウィンドウ位置、サイズ固定
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); // 完全に透明
	ImGui::Begin("Gizmo", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);
	ImGuizmo::BeginFrame();

	// ImGuizmoの設定
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::Enable(true);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

	ImGuiIO& io = ImGui::GetIO();
	float windowWidth = (float)ImGui::GetWindowWidth();
	float windowHeight = (float)ImGui::GetWindowHeight();

	// ウィンドウサイズを取得してImGuizmoに渡す
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

	// デバッグカメラ更新
	debugCamera_->Update(*input_);
	for (uint32_t i = 0; i < 5; ++i) {
		object3ds_[i]->SetViewMatrix(debugCamera_->GetViewMatrix());
	}

	for (uint32_t i = 0; i < 5; ++i) {
		transform_[i].translate = object3ds_[i]->GetTranslate();
		transform_[i].rotate = object3ds_[i]->GetRotate();
		transform_[i].scale = object3ds_[i]->GetScale();
	}

	ImGuizmo::Manipulate(
	    *(debugCamera_->GetViewMatrix()).m, *(object3ds_[objIndex_]->GetProjectionMatrix()).m, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, *(object3ds_[objIndex_]->GetWorldMatrix()).m);

	ImGui::End();
	ImGui::PopStyleColor();

	// ImGui
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::Begin("Editor");

	ImGui::SliderInt("objIndex", &objIndex_, 0, 4);

	ImGui::SliderFloat4("color", &color_.x, 0.0f, 1.0f);
	object3ds_[3]->SetColor(color_);

	// object3dUpdate
	for (uint32_t i = 0; i < 5; ++i) {
		object3ds_[i]->Update();
	}

	// spriteUpdate
	for (uint32_t i = 0; i < 5; ++i) {
		sprites_[i]->Update();
	}

	// Sprite ImGui
	Vector2 position = sprites_[0]->GetPosition();
	ImGui::DragFloat2("position", &position.x, 0.1f);
	sprites_[0]->SetPosition(position);

	Vector2 size = sprites_[0]->GetSize();
	ImGui::DragFloat2("size", &size.x, 0.1f);
	sprites_[0]->SetSize(size);

	ImGui::End();
}

void Game::Draw() {
	// 描画開始
	dxCommon_->BeginFrame();

	// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spriteCommon_->DrawSettingCommon();

	// 3Dオブジェクト描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	object3dCommon_->DrawSettingCommon();

	// 3Dモデル描画
	for (uint32_t i = 0; i < 5; ++i) {
		object3ds_[i]->Draw();
	}

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