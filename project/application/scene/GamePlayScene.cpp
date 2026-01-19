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

	player_ = std::make_unique<Player>(); // プレイヤー生成
	enemy_ = std::make_unique<Enemy>(); // 敵生成
	map_ = std::make_unique<Map>(); // マップ生成

	// オーディオ初期化
	audio_ = std::make_unique<XAudio>();
	audio_->Initialize();
	audio_->SoundsAllLoad("resources/GameScene.mp3");
	audio_->SoundPlayWave();

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

	// 画面両端の幕
	rightCurtain_ = std::make_unique<BothCurtain>();
	rightCurtain_->Initialize(ctx);
	leftCurtain_ = std::make_unique<BothCurtain>();
	leftCurtain_->Initialize(ctx);

	// ヒップドロップゲージ
	hipDropGauge_ = std::make_unique<PlayerGauge>();
	hipDropGauge_->Initialize(ctx);

	// 雲
	for (auto& cloud : clouds_) {
		cloud = std::make_unique<Cloud>();
		cloud->Initialize(engineContext_);
	}

	// 目標フルーツ背景スプライトの初期化
	targetFruitBG_ = std::make_unique<Sprite>();
	targetFruitBG_->Initialize(engineContext_, "resources/white.png");
	targetFruitBG_->SetPosition({0.0f, 30.0f});
	targetFruitBG_->SetSize({1280.0f, 80.0f});
	targetFruitBG_->SetColor({0.2f, 0.2f, 0.2f, 0.5f});

	// 初期フルーツ数設定
	initialFruitCount_ = 3;

	// 木のアニメーション用タイマー初期化
	timer_ = 0.0f;

	// 目標フルーツの初期化
	GenerateTargetFruits();

	// ゲームシーン開始
	StartGameScene();

	// 数字画像集
	for (int i = 0; i < numberSprites_.size(); i++) {
		numberSprites_[i] = std::make_unique<Sprite>();
		numberSprites_[i]->Initialize(engineContext_, "resources/" + std::to_string(i) + ".png");
	}

	// ヒップドロップダメージ表示用スプライトの初期化
	for (int i = 0; i < hipDropDamageSprites_.size(); ++i) {
		hipDropDamageSprites_[i] = std::make_unique<Sprite>();
		hipDropDamageSprites_[i]->Initialize(engineContext_, "resources/1.png");
		hipDropDamageSprites_[i]->SetSize({30.0f, 50.0f});
		hipDropDamageSprites_[i]->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	}

	// プレイヤーのヒップドロップパワースプライト
	hipDropPowerSprite_ = std::make_unique<Sprite>();
	hipDropPowerSprite_->Initialize(engineContext_, "resources/attackAmount.png");
	hipDropPowerSprite_->SetPosition({1140.0f, 50.0f});

	// 砲台の生成&初期化
	gunTurret_=std::make_unique<GunTurret>();
	gunTurret_->Initialize(engineContext_);
}

void GamePlayScene::Update() {
	// 一定時間おきにフルーツ生成
	CreateGrape();
	CreateApple();
	CreateOrange();

	// 当たり判定
	CollisionPlayerEnemy();
	CollisionEnemyPlayerHipDrop();
	CollisionPlayerFruits();

	// 目標フルーツの判定と更新
	UpdateTargetFruits();

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

	// 砲台の更新
	gunTurret_->Update(timeManager_->GetDeltaTime());

	// 砲台のターゲット位置をプレイヤーの位置に設定
	gunTurret_->SetTargetPos(player_->GetTranslate());

	// 敵更新
	enemy_->Update(timeManager_->GetDeltaTime());

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
	TreeAnimation();

	// 目標フルーツ背景スプライト更新
	targetFruitBG_->Update();

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

	// ヒップドロップダメージ表示の更新
	UpdateHipDropDamageDisplay();

	// ゲーム終了判定
	EndGameCheck();

	// ヒップドロップパワースプライト更新
	hipDropPowerSprite_->Update();

#ifdef USE_IMGUI
	// ImGuiデバッグ表示
	ImGuiUpdate();
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

	// 砲台の描画
	gunTurret_->Draw();

	// 敵描画
	enemy_->Draw();

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

	// Particle
	particleDustPlayer_->Draw();
	particleDustEnemy_->Draw();

	// ヒップドロップゲージ
	hipDropGauge_->Draw();

	// 目標フルーツ背景スプライトの描画
	targetFruitBG_->Draw();

	// 目標フルーツの描画
	DrawTargetFruits();

	// 画面両端の幕
	rightCurtain_->Draw();
	leftCurtain_->Draw();

	// ヒップドロップダメージの描画
	for (auto& sprite : hipDropDamageSprites_) {
		sprite->Draw();
	}

	// ヒップドロップパワースプライト描画
	hipDropPowerSprite_->Draw();
}

void GamePlayScene::Finalize() {
	player_.reset();
	map_.reset();
	blocks_.clear();
	enemy_.reset();
	particleDustPlayer_.reset();
	particleDustEnemy_.reset();
	for (auto& tree : treeModels_) {
		tree.reset();
	}
	audio_->~XAudio();
}

void GamePlayScene::StartGameScene() {
	// 既存のブロックを破棄
	blocks_.clear();

	// マップ初期化
	map_->Initialize();

	// プレイヤー初期化
	player_->Initialize(engineContext_, keyboard_, gamePad_);
	player_->SetModel("HiyokoGlass.obj");

	// 敵初期化
	enemy_->Initialize(engineContext_);
	enemy_->SetModel("enemy.obj");

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

void GamePlayScene::ImGuiUpdate() {
	player_->UpdateImGui();
	enemy_->UpdateImGui();
	ImGuiFPS();
	ImGuiDebugCamera();
}
#endif

void GamePlayScene::CollisionPlayerEnemy() {
	if (Collision::Intersect(player_->GetAABB(), enemy_->GetAABBLeftSide()) || Collision::Intersect(player_->GetAABB(), enemy_->GetAABBRightSide())) {
		player_->SetIsHitEnemy(true);
	}
}

void GamePlayScene::CollisionEnemyPlayerHipDrop() {
	if (Collision::Intersect(player_->GetAABB(), enemy_->GetAABB()) && player_->GetHipDropDamage()) {
		enemy_->SetIsHitPlayerHipDrop(true);

		if (!player_->GetIsHitEnemyHipDrop()) {
			player_->SetIsHitEnemyHipDrop(true);
			int damage = player_->GetHipDropPowerLevel();
			enemy_->SubHP(damage);
			StartShake(30, 0.5f);
		}
	}
}

void GamePlayScene::CollisionPlayerFruits() {
	grapes_.erase(
	    std::remove_if(
	        grapes_.begin(), grapes_.end(),
	        [this](std::unique_ptr<Grape>& grape) {
		        if (Collision::Intersect(player_->GetAABB(), grape->GetAABB())) {
			        // 目標フルーツの中から未収集のブドウを探す
			        for (auto& fruit : fruitOrder_) {
				        if (fruit.fruitType == "grape" && !fruit.isCollected) {
					        fruit.isCollected = true;
					        collectedFruitCount_++;
					        // ブドウの色を変更（例：グレー）
					        fruit.sprite->SetColor({0.5f, 0.5f, 0.5f, 0.5f});
					        break;
				        }
			        }
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
			        // 目標フルーツの中から未収集のリンゴを探す
			        for (auto& fruit : fruitOrder_) {
				        if (fruit.fruitType == "apple" && !fruit.isCollected) {
					        fruit.isCollected = true;
					        collectedFruitCount_++;
					        // リンゴの色を変更（例：グレー）
					        fruit.sprite->SetColor({0.5f, 0.5f, 0.5f, 0.5f});
					        break;
				        }
			        }
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
			        // 目標フルーツの中から未収集のオレンジを探す
			        for (auto& fruit : fruitOrder_) {
				        if (fruit.fruitType == "orange" && !fruit.isCollected) {
					        fruit.isCollected = true;
					        collectedFruitCount_++;
					        // オレンジの色を変更（例：グレー）
					        fruit.sprite->SetColor({0.5f, 0.5f, 0.5f, 0.5f});
					        break;
				        }
			        }
			        player_->SetIsPowerUp(true);
			        return true;
		        }
		        return false;
	        }),
	    oranges_.end());
}

void GamePlayScene::CreateGrape() {
	if (grapes_.size() >= grapeGenerator_.kCount)
		return;

	// 生成タイマーカウント
	grapeGenerator_.generateTimer += timeManager_->GetDeltaTime();

	if (grapeGenerator_.generateTimer >= grapeGenerator_.kGenerateTimer) {
		auto grape = std::make_unique<Grape>();
		grape->Initialize(engineContext_);

		Vector3 pos;
		pos.x = RandomUtils::RangeFloat(2.0f, 30.0f);
		pos.y = RandomUtils::RangeFloat(2.0f, 10.0f);
		pos.z = 0.0f;

		grape->SetTranslate(pos);
		grapes_.push_back(std::move(grape));

		grapeGenerator_.generateTimer = 0.0f;
	}
}

void GamePlayScene::CreateApple() {
	if (grapes_.size() >= appleGenerator_.kCount)
		return;

	// 生成タイマーカウント
	appleGenerator_.generateTimer += timeManager_->GetDeltaTime();

	if (appleGenerator_.generateTimer >= appleGenerator_.kGenerateTimer) {
		auto apple = std::make_unique<Apple>();
		apple->Initialize(engineContext_);

		Vector3 pos;
		pos.x = RandomUtils::RangeFloat(2.0f, 30.0f);
		pos.y = RandomUtils::RangeFloat(2.0f, 10.0f);
		pos.z = 0.0f;

		apple->SetTranslate(pos);
		apples_.push_back(std::move(apple));

		appleGenerator_.generateTimer = 0.0f;
	}
}

void GamePlayScene::CreateOrange() {
	if (grapes_.size() >= orangeGenerator_.kCount)
		return;

	// 生成タイマーカウント
	orangeGenerator_.generateTimer += timeManager_->GetDeltaTime();

	if (orangeGenerator_.generateTimer >= orangeGenerator_.kGenerateTimer) {
		auto orange = std::make_unique<Orange>();
		orange->Initialize(engineContext_);

		Vector3 pos;
		pos.x = RandomUtils::RangeFloat(2.0f, 30.0f);
		pos.y = RandomUtils::RangeFloat(2.0f, 10.0f);
		pos.z = 0.0f;

		orange->SetTranslate(pos);
		oranges_.push_back(std::move(orange));

		orangeGenerator_.generateTimer = 0.0f;
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

void GamePlayScene::EndGameCheck() {
	if (player_->GetHP() <= 0 || enemy_->GetHP() <= 0) {
		std::string resultStatus;
		if (player_->GetHP() <= 0) {
			resultStatus = "GameOver2.obj";
		} else if (enemy_->GetHP() <= 0) {
			resultStatus = "StageClear.obj";
		}

		// 結果文字列をファイルに保存
		SaveResultStatus(resultStatus);
		RequestSceneChange("Result");
	}
}

void GamePlayScene::TreeAnimation() {
	timer_ += timeManager_->GetDeltaTime();
	t_ = timer_ / kTimer;
	t_ = std::clamp(t_, 0.0f, 1.0f);

	float x = Easing::EaseInOutBack(t_) * 5.0f;

	for (auto& tree : treeModels_) {
		tree->SetScale({x + 1.0f, x + 1.0f, x + 1.0f});
		tree->SetRotate({0.0f, x * std::numbers::pi_v<float> * 2.0f, 0.0f});
	}

	for (auto& tree : treeModels_) {
		tree->Update();
	}
}

void GamePlayScene::UpdateHipDropDamageDisplay() {
	int hipDropDamage = player_->GetHipDropPowerLevel();
	// 10の位
	int digit10 = hipDropDamage / 10;
	// 1の位
	int digit1 = hipDropDamage % 10;

	if (digit10 > 0) { // 10の位がある場合のみ表示
		hipDropDamageSprites_[0]->SetTexture("resources/" + std::to_string(digit10) + ".png");
		hipDropDamageSprites_[0]->SetPosition({1150.0f, 100.0f}); // 位置調整
		hipDropDamageSprites_[0]->SetSize({30.0f, 50.0f});
		hipDropDamageSprites_[0]->Update();
	} else {
		hipDropDamageSprites_[0]->SetSize({0.0f, 0.0f}); // 10の位が0の場合は表示しない
	}

	hipDropDamageSprites_[1]->SetTexture("resources/" + std::to_string(digit1) + ".png");
	hipDropDamageSprites_[1]->SetPosition({1180.0f, 100.0f}); // 位置調整
	hipDropDamageSprites_[1]->Update();
}

void GamePlayScene::GenerateTargetFruits() {
	fruitOrder_.clear();
	collectedFruitCount_ = 0;

	const std::vector<std::string> fruitTypes = {"grape", "apple", "orange"};

	// initialFruitCount_個のランダムなフルーツを生成
	for (int i = 0; i < initialFruitCount_; ++i) {
		int randomIndex = RandomUtils::RangeInt(0, static_cast<int>(fruitTypes.size()) - 1);
		std::string fruitType = fruitTypes[randomIndex];

		auto newSprite = std::make_unique<Sprite>();
		newSprite->Initialize(engineContext_, "resources/" + fruitType + ".png");
		newSprite->SetSize({42.0f, 42.0f});
		fruitOrder_.push_back({fruitType, false, std::move(newSprite)});
	}
}

void GamePlayScene::DrawTargetFruits() {
	// 画面上部にUIとして目標フルーツを表示
	float startX = 640.0f - (fruitOrder_.size() - 2) * 21.0f;
	float startY = 40.0f;
	float spacing = 42.0f;

	for (size_t i = 0; i < fruitOrder_.size(); ++i) {
		auto& fruit = fruitOrder_[i];
		auto& sprite = fruit.sprite;

		sprite->SetPosition({startX + i * spacing, startY});

		// 収集済みの場合は薄い色、未収集の場合は通常の色
		if (fruit.isCollected) {
			sprite->SetColor({0.5f, 0.5f, 0.5f, 0.5f}); // グレーアウト
		} else {
			sprite->SetColor({1.0f, 1.0f, 1.0f, 1.0f}); // 通常の色
		}

		sprite->Update();
		sprite->Draw();
	}
}

void GamePlayScene::UpdateTargetFruits() {
	// すべてのフルーツが収集されたかチェック
	bool allCollected = true;
	for (const auto& fruit : fruitOrder_) {
		if (!fruit.isCollected) {
			allCollected = false;
			break;
		}
	}

	// すべて収集された場合、新しい目標を生成
	if (allCollected && fruitOrder_.size() > 0) {
		initialFruitCount_++; // 目標フルーツ数を増やす

		initialFruitCount_ = std::min(initialFruitCount_, kMaxFruitCount_); // 最大数を超えないように制限

		player_->IncrementHipDropPowerLevel(); // プレイヤーのヒップドロップパワーを上昇
		GenerateTargetFruits();
	}
}