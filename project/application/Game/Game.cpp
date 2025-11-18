#include "Game.h"
#include "Collision.h"
#include "Random.h"

    void
    Game::Initialize(HINSTANCE hInstance) {
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

	// TestParticleCommon
	particleCommon_->Initialize(dxCommon_.get());

	// .objファイルからモデルを読み込む
	AllModelLoader();

	// XAudio
	audio_->Initialize();
	audio_->SoundsAllLoad();

	// DirectInput
	input_->Initialize(winApp_.get());

	// DebugCamera
	debugCamera_->Initialize();
	debugCamera_->SetTranslation({19.45f, 10.5f, -75.0f});
	object3dCommon_->SetDefaultCamera(debugCamera_.get());
	particleCommon_->SetDefaultCamera(debugCamera_.get());

	// Particle
	particle_->Initialize(particleCommon_.get(), textureManager_.get(), modelManger_.get());

	currentScene_ = Scene::Title;
}

void Game::ChangeScene(Scene newScene) {
	if (currentScene_ == newScene)
		return;

	currentScene_ = newScene;
	if (currentScene_ == Scene::Game) {
		StartGameScene();
	}

	// 遷移時処理はここに追加

}

void Game::StartGameScene() {
	// 既存のブロックを破棄
	blocks_.clear();

	// マップ初期化
	map_->Initialize();

	// プレイヤー初期化
	player_->Initialize(object3dCommon_.get(), textureManager_.get(), modelManger_.get(), input_.get(), gamePad_.get(), spriteCommon_.get());
	player_->SetModel("Box.obj");

	// 敵初期化
	enemy_->Initialize(object3dCommon_.get(), textureManager_.get(), modelManger_.get(), spriteCommon_.get());
	enemy_->SetModel("sphere.obj");

	// パワーアップアイテム初期化
	//powerUpItems_.clear();
	//powerUpItemCreateFrameCount_ = 0;

	// オブジェクトの配置
	SpawnObjectsByMapChip(mapLeftTop_);
}

void Game::Update() {
	// シーンに依らず、経過時間と入力は更新しておく
	deltaTime_->Update();
	fps_ = 1.0f / deltaTime_->GetDeltaTime();
	input_->Update();

#ifdef USE_IMGUI
	// ImGui前処理
	imGuiManager_->BeginFrame();
#endif

	// シーン毎の処理
	if (currentScene_ == Scene::Title) {
		// タイトル
#ifdef USE_IMGUI
		ImGui::Begin("Title");
		ImGui::TextWrapped("Title Scene");
		ImGui::TextWrapped("Press Space or GamePad A to Start");
		ImGui::End();
#endif
		bool startInput = input_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;
		if (startInput) {
			ChangeScene(Scene::Game);
		}

		// デバッグカメラ更新
		debugCamera_->Update(*input_, *gamePad_);
		gamePad_->Update();
	} else if (currentScene_ == Scene::Game) {
		// ゲームシーン
#ifdef USE_IMGUI
		//player_->UpdateImGui();
		//enemy_->UpdateImGui();
		// ImGuiFPS();
		//ImGuiDebugCamera();

		ImGui::Begin("Rule");

		ImGui::Text("Move with the AD or stick");
		ImGui::Text("Press Space or A button while on the ground to Jump");
		ImGui::Text("Press Space or A button in midair to perform a Hip Drop");

		ImGui::End();
#endif

		// 一定時間おきにパワーアップアイテム生成
		//CreatePowerUpItem();

		// デバッグカメラ更新
		debugCamera_->Update(*input_, *gamePad_);

		// 当たり判定
		CollisionPlayerEnemy();
		CollisionEnemyPlayerHipDrop();
		//CollisionPlayerPowerUpItem();

		// プレイヤー更新
		player_->Update(deltaTime_->GetDeltaTime());

		// ブロック更新
		for (auto& block : blocks_) {
			block->Update();
		}

		// 敵更新
		enemy_->Update(deltaTime_->GetDeltaTime());

		// パワーアップアイテム更新
		//for (auto& powerUpItem : powerUpItems_) {
		//	powerUpItem->Update(deltaTime_->GetDeltaTime());
		//}

		// Particle
		particle_->Update();

		// ゲーム終了判定
		if (player_->GetHP() <= 0 || enemy_->GetHP() <= 0) {
			ChangeScene(Scene::Result);
		}
	} else if (currentScene_ == Scene::Result) {
		// リザルト
#ifdef USE_IMGUI
		ImGui::Begin("Result");
		ImGui::TextWrapped("Result Scene");
		ImGui::TextWrapped("Press Space or GamePad A to Return to Title");
		ImGui::End();
#endif
		bool backInput = input_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;
		if (backInput) {
			ChangeScene(Scene::Title);
		}

		// デバッグカメラ更新
		debugCamera_->Update(*input_, *gamePad_);
		gamePad_->Update();
	}

#ifdef USE_IMGUI
	// ImGui のフレームは EndFrame 時に描画される（Draw内で Render を呼ぶ）
#endif
}

void Game::Draw() {
	// 描画開始
	dxCommon_->BeginFrame();

	srvManager_->PreDraw();

	// シーンごとの描画
	if (currentScene_ == Scene::Game) {
		// プレイヤー描画
		player_->Draw();

		// ブロック描画
		for (auto& block : blocks_) {
			block->Draw();
		}

		// 敵描画
		enemy_->Draw();

		// パワーアップアイテム描画
		//for (auto& powerUpItem : powerUpItems_) {
		//	powerUpItem->Draw();
		//}

		// Particle
		particle_->Draw();
	} else {
	}

	// ImGuiの内部コマンドを生成する
#ifdef USE_IMGUI
	imGuiManager_->Render(dxCommon_->GetCommandList());
#endif

	// 描画終了
	dxCommon_->EndFrame();
}

void Game::Finalize() {
#ifdef USE_IMGUI
	imGuiManager_->Finalize();
#endif

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
				block->Spawn({static_cast<float>(x) - 11.0f, static_cast<float>(y) * -1.0f + 19.0f, 0.0f});
				block->SetModel("Box.obj");
				blocks_.push_back(std::move(block));
			}
		}
	}
}

void Game::ImGuiFPS() {
#ifdef USE_IMGUI
	ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("FPS: %.1f", fps_);
	ImGui::Text("Frame Time: %.3f ms", deltaTime_->GetDeltaTime() * 1000.0f);
	ImGui::End();
#endif
}

void Game::ImGuiDebugCamera() {
#ifdef USE_IMGUI
	ImGui::Begin("DebugCamera");
	ImGui::DragFloat3("Position", &debugCamera_->GetTranslation().x, 0.01f);
	ImGui::End();
#endif
}

void Game::CollisionPlayerEnemy() {
	if (Collision::Intersect(player_->GetAABB(), enemy_->GetAABBLeftSide()) || Collision::Intersect(player_->GetAABB(), enemy_->GetAABBRightSide())) {
		// 敵に当たった時のフラグを立てる
		player_->SetIsHitEnemy(true);
	}
}

void Game::CollisionEnemyPlayerHipDrop() {
	if (Collision::Intersect(player_->GetAABB(), enemy_->GetAABB())) {
		// プレイヤーからヒップドロップを受けた時のフラグを立てる
		enemy_->SetIsHitPlayerHipDrop(true);

		// プレイヤーを反動で打ち上げる
		if (!player_->GetIsHitEnemyHipDrop()) {
			player_->SetIsHitEnemyHipDrop(true);
		}
	}
}

void Game::CollisionPlayerPowerUpItem() {
	// パワーアップアイテムとの衝突判定と削除
	powerUpItems_.erase(
	    std::remove_if(
	        powerUpItems_.begin(), powerUpItems_.end(),
	        [this](std::unique_ptr<PowerUpItem>& item) {
		        if (Collision::Intersect(player_->GetAABB(), item->GetAABB())) {
			        // プレイヤーのパワーアップフラグを立てる
			        player_->SetIsPowerUp(true);

			        return true;
		        }
		        return false;
	        }),
	    powerUpItems_.end());
}

void Game::AllModelLoader() {
	modelManger_->LoadModel("fence.obj");
	modelManger_->LoadModel("plane.obj");
	modelManger_->LoadModel("axis.obj");
	modelManger_->LoadModel("SkySphere.obj");
	modelManger_->LoadModel("skydome.obj");
	modelManger_->LoadModel("Field.obj");
	modelManger_->LoadModel("sphere.obj");
	modelManger_->LoadModel("Box.obj");
}

void Game::CreatePowerUpItem() {
	// パワーアップアイテムの数が最大値に達していたら早期リターン
	if (powerUpItems_.size() >= kPowerUpItemCountMax)
		return;

	// フレームカウント加算
	powerUpItemCreateFrameCount_++;

	// フレームカウントが上限に達したら
	if (powerUpItemCreateFrameCount_ >= kPowerUpItemFrameCountMax) {
		// パワーアップアイテム生成
		auto powerUpItem = std::make_unique<PowerUpItem>();
		powerUpItem->Initialize(object3dCommon_.get(), textureManager_.get(), modelManger_.get());
		powerUpItem->SetModel("sphere.obj");

		// ランダムな座標を指定
		Vector3 pos;
		pos.x = RandomUtils::RangeFloat(2.0f, 30.0f);
		pos.y = RandomUtils::RangeFloat(2.0f, 20.0f);
		pos.z = 0.0f;

		powerUpItem->SetTranslate(pos);
		powerUpItems_.push_back(std::move(powerUpItem));

		// フレームカウントをリセット
		powerUpItemCreateFrameCount_ = 0;

		return;
	}
}