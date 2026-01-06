#include "GamePlayScene.h"
#include "Collision.h"
#include "Easing.h"
#include "Random.h"
#include "SceneManager.h"
#include <algorithm>
#include <fstream>

void GamePlayScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	player_ = std::make_unique<Player>();
	enemy_ = std::make_unique<Enemy>();
	map_ = std::make_unique<Map>();

	// 土埃パーティクル プレイヤー用
	particleDustPlayer_ = std::make_unique<Particle>();
	particleDustPlayer_->Initialize(engineContext_, {30.0f, 10.0f, 0.0f}, "resources/smoke.png", 3, "Dust");

	// 土埃パーティクル 敵用
	particleDustEnemy_ = std::make_unique<Particle>();
	particleDustEnemy_->Initialize(engineContext_, {30.0f, 10.0f, 0.0f}, "resources/smoke.png", 4, "Dust");

	// 木のモデル
	for (int i = 0; i < treeModels_.size(); i++) {
		auto tree = std::make_unique<Object3d>();
		tree->Initialize(engineContext_);
		tree->SetModel("tree.obj");
		tree->SetTranslate({i * 7.0f - 5.0f, 0.0f, 8.0f});
		tree->SetScale({3.0f, 3.0f, 3.0f});
		tree->Update();
		treeModels_[i] = std::move(tree);
	}

	// 地形モデル
	terrainModel_ = std::make_unique<Object3d>();
	terrainModel_->Initialize(engineContext_);
	terrainModel_->SetModel("terrain.obj");
	terrainModel_->SetTranslate({20.0f, 0.0f, 0.0f});
	terrainModel_->SetScale({1.0f, 1.0f, 1.0f});
	terrainModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});

	// 画面両端の幕
	rightCurtain_ = std::make_unique<BothCurtain>();
	rightCurtain_->Initialize(ctx);
	leftCurtain_ = std::make_unique<BothCurtain>();
	leftCurtain_->Initialize(ctx);

	// ヒップドロップゲージ
	hipDropGauge_ = std::make_unique<PlayerGauge>();
	hipDropGauge_->Initialize(ctx);

	// フィールドモデル
	/*fieldModel_ = std::make_unique<Object3d>();
	fieldModel_->Initialize(engineContext_);
	fieldModel_->SetModel("field1.obj");
	fieldModel_->SetTranslate({20.0f, 0.0f, 0.0f});
	fieldModel_->SetScale({4.0f, 4.0f, 4.0f});
	fieldModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});*/

	// 雲
	for (auto& cloud : clouds_) {
		cloud = std::make_unique<Cloud>();
		cloud->Initialize(engineContext_);
	}

	// ゲームシーン開始
	StartGameScene();
}

void GamePlayScene::Update() {
	// 一定時間おきにパワーアップアイテム生成
	CreatePowerUpItem();

	// 一定時間おきにフルーツ生成
	CreateGrape();
	CreateApple();
	CreateOrange();

	// 当たり判定
	CollisionPlayerEnemy();
	CollisionEnemyPlayerHipDrop();
	CollisionPlayerPowerUpItem();
	CollisionPlayerFruits();

	// プレイヤー更新
	player_->Update(timeManager_->GetDeltaTime());

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
	enemy_->Update(timeManager_->GetDeltaTime());

	// フィールドモデル
	// fieldModel_->Update();

	// パワーアップアイテム更新
	/*for (auto& powerUpItem : powerUpItems_) {
	    powerUpItem->Update(timeManager_->GetDeltaTime());
	}*/

	// 雲
	for (auto& cloud : clouds_) {
		cloud->Update(timeManager_->GetDeltaTime());
	}

	// ブドウ更新
	for (auto& grape : grapes_) {
		grape->Update(timeManager_->GetDeltaTime());
	}

	// リンゴ更新
	for (auto& apple : apples_) {
		apple->Update(timeManager_->GetDeltaTime());
	}

	// オレンジ更新
	for (auto& orange : oranges_) {
		orange->Update(timeManager_->GetDeltaTime());
	}

	// 木のモデル
	{
		timer_ += timeManager_->GetDeltaTime();
		t_ = timer_ / kTimer;
		t_ = std::clamp(t_, 0.0f, 1.0f);

		float x = Easing::easeInOutBack(t_) * 5.0f;

		for (auto& tree : treeModels_) {
			tree->SetScale({x + 1.0f, x + 1.0f, x + 1.0f});
			tree->SetRotate({0.0f, x * std::numbers::pi_v<float> * 2.0f, 0.0f});
		}

		for (auto& tree : treeModels_) {
			tree->Update();
		}
	}

	terrainModel_->Update();

	// カメラシェイク
	ShakeCamera();

	// Particle
	particleDustPlayer_->Update();
	particleDustEnemy_->Update();

	// 画面両端の幕
	rightCurtain_->Update();
	leftCurtain_->Update();

	// ヒップドロップゲージ
	hipDropGauge_->Update();

	// ゲーム終了判定
	if (player_->GetHP() <= 0 || enemy_->GetHP() <= 0) {
		std::string resultStatus;
		if (player_->GetHP() <= 0) {
			resultStatus = "GameOver2.obj";
		} else if (enemy_->GetHP() <= 0) {
			resultStatus = "StageClear.obj";
		}

		// 結果文字列をファイルに保存
		SaveResultStatus(resultStatus);
		sceneManager_->ChangeScene("Result");
	}

#ifdef USE_IMGUI
	player_->UpdateImGui();
	enemy_->UpdateImGui();
	ImGuiFPS();
	ImGuiDebugCamera();

	ImGui::Begin("PointLight");
	ImGui::DragFloat3("position", &terrainModel_->GetSpotLight().position.x, 0.1f);
	ImGui::DragFloat("intensity", &terrainModel_->GetSpotLight().intensity, 0.01f);
	ImGui::DragFloat("distance", &terrainModel_->GetSpotLight().distance, 0.01f);
	ImGui::DragFloat("decay", &terrainModel_->GetSpotLight().decay, 0.01f);
	ImGui::DragFloat("cos", &terrainModel_->GetSpotLight().cosAngle, 0.01f);
	ImGui::End();
#endif

#ifdef _DEBUG
	// デバッグ用: プレイヤーのHPを0にする
	if (keyboard_->KeyTriggered(DIK_Z)) {
		player_->SetIsHitEnemy(true);
	}
	// デバッグ用: 敵のHPを0にする
	if (keyboard_->KeyTriggered(DIK_X)) {
		enemy_->SetIsHitPlayerHipDrop(true);
	}
#endif
}

void GamePlayScene::Draw() {
	// プレイヤー描画
	player_->Draw();

	// ブロック描画
	for (auto& block : blocks_) {
		block->Draw();
	}

	// 敵描画
	enemy_->Draw();

	// フィールドモデル
	// fieldModel_->Draw();

	// パワーアップアイテム描画
	/*for (auto& powerUpItem : powerUpItems_) {
	    powerUpItem->Draw();
	}*/

	// 雲
	for (auto& cloud : clouds_) {
		cloud->Draw();
	}

	// ブドウ描画
	for (auto& grape : grapes_) {
		grape->Draw();
	}

	// リンゴ描画
	for (auto& apple : apples_) {
		apple->Draw();
	}

	// オレンジ描画
	for (auto& orange : oranges_) {
		orange->Draw();
	}

	// 木のモデル
	for (auto& tree : treeModels_) {
		tree->Draw();
	}

	terrainModel_->Draw();

	// Particle
	particleDustPlayer_->Draw();
	particleDustEnemy_->Draw();

	// 画面両端の幕
	rightCurtain_->Draw();
	leftCurtain_->Draw();

	// ヒップドロップゲージ
	hipDropGauge_->Draw();
}

void GamePlayScene::Finalize() {
	player_.reset();
	map_.reset();
	blocks_.clear();
	enemy_.reset();
	powerUpItems_.clear();
	particleDustPlayer_.reset();
	particleDustEnemy_.reset();
	for (auto& tree : treeModels_) {
		tree.reset();
	}
	terrainModel_.reset();
}

void GamePlayScene::StartGameScene() {
	// 既存のブロックを破棄
	blocks_.clear();

	// マップ初期化
	map_->Initialize();

	// プレイヤー初期化
	player_->Initialize(engineContext_, keyboard_, gamePad_);
	player_->SetModel("Hiyoko.obj");

	// 敵初期化
	enemy_->Initialize(engineContext_);
	enemy_->SetModel("sphere.obj");

	// パワーアップアイテム初期化
	powerUpItems_.clear();
	powerUpItemCreateFrameCount_ = 0;

	// オブジェクトの配置
	SpawnObjectsByMapChip(mapLeftTop_);
}

void GamePlayScene::SpawnObjectsByMapChip(Vector2 leftTop) {
	for (int y = 0; y < map_->GetRowCount(); ++y) {
		for (int x = 0; x < map_->GetColumnCount(); ++x) {
			int tile = map_->GetMapData(y, x);

			// タイルごとの描画処理
			if (static_cast<TileType>(tile) == TileType::BLOCK) {
				// ブロックの初期化
				auto block = std::make_unique<Block>();
				block->Initialize(engineContext_);
				block->Spawn({static_cast<float>(x) - 11.0f, static_cast<float>(y) * -1.0f + 19.0f, 0.0f});
				block->SetModel("Box.obj");
				blocks_.push_back(std::move(block));
			}
		}
	}
}

#ifdef USE_IMGUI
void GamePlayScene::ImGuiFPS() {
	ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("FPS: %.3f", 1.0f / timeManager_->GetDeltaTime());
	ImGui::Text("Frame Time: %.3f ms", timeManager_->GetDeltaTime() * 1000.0f);
	ImGui::End();
}

void GamePlayScene::ImGuiDebugCamera() {
	ImGui::Begin("DebugCamera");
	ImGui::DragFloat3("Position", &debugCamera_->GetTranslation().x, 0.01f);
	ImGui::End();
}
#endif

void GamePlayScene::CollisionPlayerEnemy() {
	if (Collision::Intersect(player_->GetAABB(), enemy_->GetAABBLeftSide()) || Collision::Intersect(player_->GetAABB(), enemy_->GetAABBRightSide())) {
		player_->SetIsHitEnemy(true);
	}
}

void GamePlayScene::CollisionEnemyPlayerHipDrop() {
	if (Collision::Intersect(player_->GetAABB(), enemy_->GetAABB())) {
		enemy_->SetIsHitPlayerHipDrop(true);

		if (!player_->GetIsHitEnemyHipDrop()) {
			player_->SetIsHitEnemyHipDrop(true);
			StartShake(30, 0.5f);
		}
	}
}

void GamePlayScene::CollisionPlayerPowerUpItem() {
	powerUpItems_.erase(
	    std::remove_if(
	        powerUpItems_.begin(), powerUpItems_.end(),
	        [this](std::unique_ptr<PowerUpItem>& item) {
		        if (Collision::Intersect(player_->GetAABB(), item->GetAABB())) {
			        player_->SetIsPowerUp(true);
			        return true;
		        }
		        return false;
	        }),
	    powerUpItems_.end());
}

void GamePlayScene::CollisionPlayerFruits() {
	grapes_.erase(
	    std::remove_if(
	        grapes_.begin(), grapes_.end(),
	        [this](std::unique_ptr<Grape>& grape) {
		        if (Collision::Intersect(player_->GetAABB(), grape->GetAABB())) {
			        player_->SetIsPowerUp(true);
			        return true;
		        }
		        return false;
	        }),
	    grapes_.end());

	apples_.erase(
	    std::remove_if(
	        apples_.begin(), apples_.end(),
	        [this](std::unique_ptr<Apple>& apple) {
		        if (Collision::Intersect(player_->GetAABB(), apple->GetAABB())) {
			        player_->SetIsPowerUp(true);
			        return true;
		        }
		        return false;
	        }),
	    apples_.end());

	oranges_.erase(
	    std::remove_if(
	        oranges_.begin(), oranges_.end(),
	        [this](std::unique_ptr<Orange>& orange) {
		        if (Collision::Intersect(player_->GetAABB(), orange->GetAABB())) {
			        player_->SetIsPowerUp(true);
			        return true;
		        }
		        return false;
	        }),
	    oranges_.end());
}

void GamePlayScene::CreatePowerUpItem() {
	if (powerUpItems_.size() >= kPowerUpItemCountMax)
		return;

	powerUpItemCreateFrameCount_++;

	if (powerUpItemCreateFrameCount_ >= kPowerUpItemFrameCountMax) {
		auto powerUpItem = std::make_unique<PowerUpItem>();
		powerUpItem->Initialize(engineContext_);

		Vector3 pos;
		pos.x = RandomUtils::RangeFloat(2.0f, 30.0f);
		pos.y = RandomUtils::RangeFloat(2.0f, 20.0f);
		pos.z = 0.0f;

		powerUpItem->SetTranslate(pos);
		powerUpItems_.push_back(std::move(powerUpItem));

		powerUpItemCreateFrameCount_ = 0;
	}
}

void GamePlayScene::CreateGrape() {
	if (grapes_.size() >= grapeGenarator_.kCount)
		return;

	// 生成タイマーカウント
	grapeGenarator_.generateTimer += timeManager_->GetDeltaTime();

	if (grapeGenarator_.generateTimer >= grapeGenarator_.kGenerateTimer) {
		auto grape = std::make_unique<Grape>();
		grape->Initialize(engineContext_);

		Vector3 pos;
		pos.x = RandomUtils::RangeFloat(2.0f, 30.0f);
		pos.y = RandomUtils::RangeFloat(2.0f, 20.0f);
		pos.z = 0.0f;

		grape->SetTranslate(pos);
		grapes_.push_back(std::move(grape));

		grapeGenarator_.generateTimer = 0.0f;
	}
}

void GamePlayScene::CreateApple() {
	if (grapes_.size() >= appleGenarator_.kCount)
		return;

	// 生成タイマーカウント
	appleGenarator_.generateTimer += timeManager_->GetDeltaTime();

	if (appleGenarator_.generateTimer >= appleGenarator_.kGenerateTimer) {
		auto apple = std::make_unique<Apple>();
		apple->Initialize(engineContext_);

		Vector3 pos;
		pos.x = RandomUtils::RangeFloat(2.0f, 30.0f);
		pos.y = RandomUtils::RangeFloat(2.0f, 20.0f);
		pos.z = 0.0f;

		apple->SetTranslate(pos);
		apples_.push_back(std::move(apple));

		appleGenarator_.generateTimer = 0.0f;
	}
}

void GamePlayScene::CreateOrange() {
	if (grapes_.size() >= orangeGenarator_.kCount)
		return;

	// 生成タイマーカウント
	orangeGenarator_.generateTimer += timeManager_->GetDeltaTime();

	if (orangeGenarator_.generateTimer >= orangeGenarator_.kGenerateTimer) {
		auto orange = std::make_unique<Orange>();
		orange->Initialize(engineContext_);

		Vector3 pos;
		pos.x = RandomUtils::RangeFloat(2.0f, 30.0f);
		pos.y = RandomUtils::RangeFloat(2.0f, 20.0f);
		pos.z = 0.0f;

		orange->SetTranslate(pos);
		oranges_.push_back(std::move(orange));

		orangeGenarator_.generateTimer = 0.0f;
	}
}

void GamePlayScene::StartShake(int frames, float magnitude) {
	isShake_ = true;
	shakeFrames_ = frames;
	currentFrame_ = 0;
	magnitude_ = magnitude;
	originalPos_ = debugCamera_->GetTranslation();
}

void GamePlayScene::ShakeCamera() {
	if (!isShake_)
		return;

	const float decay = 0.9f;

	float offsetX = RandomUtils::RangeFloat(-1, 1) * magnitude_;
	float offsetY = RandomUtils::RangeFloat(-1, 1) * magnitude_;

	debugCamera_->SetTranslation({originalPos_.x + offsetX, originalPos_.y + offsetY, originalPos_.z});

	magnitude_ *= decay;
	currentFrame_++;

	if (currentFrame_ >= shakeFrames_) {
		debugCamera_->SetTranslation(originalPos_);
		isShake_ = false;
	}
}

void GamePlayScene::SaveResultStatus(const std::string& status) {
	std::ofstream file("resources/result_status.txt");
	if (file.is_open()) {
		file << status;
		file.close();
	}
}