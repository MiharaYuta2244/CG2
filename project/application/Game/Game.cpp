#include "Game.h"
#include "Collision.h"
#include "Random.h"

void Game::Initialize() {
	// 基底クラスの初期化処理
	Framework::Initialize();

	// 土埃パーティクル プレイヤー用
	particleDustPlayer_ = std::make_unique<Particle>();
	particleDustPlayer_->Initialize(&GetEngineContext(), {30.0f, 10.0f, 0.0f}, "resources/smoke.png", 3, "Dust");

	// 土埃パーティクル 敵用
	particleDustEnemy_ = std::make_unique<Particle>();
	particleDustEnemy_->Initialize(&GetEngineContext(), {30.0f, 10.0f, 0.0f}, "resources/smoke.png", 4, "Dust");

	// タイトルテキストモデル
	titleText_ = std::make_unique<Object3d>();
	titleText_->Initialize(&GetEngineContext());
	titleText_->SetModel("Title.obj");
	titleText_->SetScale({7.0f, 7.0f, 7.0f});
	titleRotateY_ = std::numbers::pi_v<float>;
	titleText_->SetRotate({0.0f, titleRotateY_, 0.0f});
	titleText_->SetTranslate({20.0f, 8.0f, 0.0f});

	// シェイクフラグ
	isShake_ = false;

	// 現在のシーン初期化
	currentScene_ = Scene::Title;
}

void Game::Update() {
	// 基底クラスの更新処理
	Framework::Update();

	// シーン毎の処理
	if (currentScene_ == Scene::Title) {
		// タイトル
#ifdef USE_IMGUI
		ImGui::Begin("Title");
		ImGui::TextWrapped("Title Scene");
		ImGui::TextWrapped("Press Space or GamePad A to Start");
		ImGui::End();

		ImGui::Begin("TitleModel");
		ImGui::DragFloat3("Position", &titleText_->GetTranslate().x, 0.01f);
		ImGui::End();
#endif
		bool startInput = GetKeyboard()->KeyTriggered(DIK_SPACE) || GetGamePad()->GetState().buttonsPressed.a;
		if (startInput) {
			ChangeScene(Scene::Game);
		}

		// タイトルテキストモデル
		titleRotateY_ += 0.02f;

		// 周期的な上下動
		float bounceY = 8.0f + sinf(titleRotateY_) * 1.0f;

		// スケールの脈動
		float scalePulse = 7.0f + 0.5f * sinf(titleRotateY_ * 2.0f);

		// 複合回転（X軸に小さな揺れ）
		float rotateX = sinf(titleRotateY_ * 0.5f) * 0.1f;

		// 反映
		titleText_->SetTranslate({20.0f, bounceY, 0.0f});
		titleText_->SetRotate({rotateX, titleRotateY_, 0.0f});
		titleText_->SetScale({scalePulse, scalePulse, scalePulse});
		titleText_->Update();

	} else if (currentScene_ == Scene::Game) {
		// ゲームシーン
#ifdef USE_IMGUI
		player_->UpdateImGui();
		enemy_->UpdateImGui();
		ImGuiFPS();
		ImGuiDebugCamera();
#endif

		// 一定時間おきにパワーアップアイテム生成
		CreatePowerUpItem();

		// 当たり判定
		CollisionPlayerEnemy();
		CollisionEnemyPlayerHipDrop();
		CollisionPlayerPowerUpItem();

		// プレイヤー更新
		player_->Update(GetTimeManager()->GetDeltaTime());

		// プレイヤーの足元に埃のパーティクルを生成するように位置を設定
		Vector3 particlePos = {player_->GetTranslate().x, player_->GetTranslate().y - player_->GetScale().y, player_->GetTranslate().z};
		particleDustPlayer_->SetTranslate(particlePos);

		// 敵の足元に埃のパーティクルを生成するように位置を設定
		particlePos = {enemy_->GetTranslate().x, enemy_->GetTranslate().y - enemy_->GetScale().y, enemy_->GetTranslate().z};
		particleDustEnemy_->SetTranslate(particlePos);

		// ブロック更新
		for (auto& block : blocks_) {
			block->Update();
		}

		// 敵更新
		enemy_->Update(GetTimeManager()->GetDeltaTime());

		// パワーアップアイテム更新
		for (auto& powerUpItem : powerUpItems_) {
			powerUpItem->Update(GetTimeManager()->GetDeltaTime());
		}

		// カメラシェイク
		ShakeCamera();

		// Particle
		particleDustPlayer_->Update();
		particleDustEnemy_->Update();

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
		bool backInput = GetKeyboard()->KeyTriggered(DIK_SPACE) || GetGamePad()->GetState().buttonsPressed.a;
		if (backInput) {
			ChangeScene(Scene::Title);
		}
	}

#ifdef USE_IMGUI
	// ImGuiのフレームはEndFrame時に描画される
#endif
}

void Game::Draw() {
	// 描画前処理
	Framework::PreDraw();

	// シーンごとの描画
	if (currentScene_ == Scene::Title) {
		// タイトルテキストモデル
		titleText_->Draw();

	} else if (currentScene_ == Scene::Game) {
		// プレイヤー描画
		player_->Draw();

		// ブロック描画
		for (auto& block : blocks_) {
			block->Draw();
		}

		// 敵描画
		enemy_->Draw();

		// パワーアップアイテム描画
		for (auto& powerUpItem : powerUpItems_) {
			powerUpItem->Draw();
		}

		// Particle
		particleDustPlayer_->Draw();
		particleDustEnemy_->Draw();

	} else {
	}

	// 描画後処理
	Framework::PostDraw();
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
	player_->Initialize(&GetEngineContext(), GetKeyboard(), GetGamePad());
	player_->SetModel("Hiyoko.obj");

	// 敵初期化
	enemy_->Initialize(&GetEngineContext());
	enemy_->SetModel("sphere.obj");

	// パワーアップアイテム初期化
	powerUpItems_.clear();
	powerUpItemCreateFrameCount_ = 0;

	// オブジェクトの配置
	SpawnObjectsByMapChip(mapLeftTop_);
}

void Game::Finalize() {
	// 基底クラスの終了処理
	Framework::Finalize();
}

void Game::SpawnObjectsByMapChip(Vector2 leftTop) {
	for (int y = 0; y < map_->GetRowCount(); ++y) {
		for (int x = 0; x < map_->GetColumnCount(); ++x) {
			int tile = map_->GetMapData(y, x);

			// タイルごとの描画処理
			if (static_cast<TileType>(tile) == TileType::BLOCK) {
				// ブロックの初期化
				auto block = std::make_unique<Block>();
				block->Initialize(&GetEngineContext());
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
	ImGui::Text("FPS: %.1f", GetFPS());
	ImGui::Text("Frame Time: %.3f ms", GetTimeManager()->GetDeltaTime() * 1000.0f);
	ImGui::End();
#endif
}

void Game::ImGuiDebugCamera() {
#ifdef USE_IMGUI
	ImGui::Begin("DebugCamera");
	ImGui::DragFloat3("Position", &GetDebugCamera()->GetTranslation().x, 0.01f);
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

			// シェイクフラグを立てる
			StartShake(30, 0.5f);
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
		powerUpItem->Initialize(&GetEngineContext());
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

void Game::StartShake(int frames, float magnitude) {
	isShake_ = true;
	shakeFrames_ = frames;
	currentFrame_ = 0;
	magnitude_ = magnitude;
	originalPos_ = GetDebugCamera()->GetTranslation();
}

void Game::ShakeCamera() {
	if (!isShake_)
		return;

	// 減衰率（指数的に収束）
	const float decay = 0.9f;

	// ランダムオフセット
	float offsetX = RandomUtils::RangeFloat(-1, 1) * magnitude_;
	float offsetY = RandomUtils::RangeFloat(-1, 1) * magnitude_;

	GetDebugCamera()->SetTranslation({originalPos_.x + offsetX, originalPos_.y + offsetY, originalPos_.z});

	// 減衰
	magnitude_ *= decay;
	currentFrame_++;

	// 規定フレーム経過したら終了
	if (currentFrame_ >= shakeFrames_) {
		GetDebugCamera()->SetTranslation(originalPos_);
		isShake_ = false;
	}
}
