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
	enemy_ = std::make_unique<Enemy>();   // 敵生成
	map_ = std::make_unique<Map>();       // マップ生成

	// オーディオ初期化
	audio_ = std::make_unique<XAudio>();
	audio_->Initialize();
	audio_->LoadWave("BGM_GameScene", "resources/GameScene.mp3");
	audio_->LoadWave("SE_HitBoss", "resources/HitBoss.mp3");
	audio_->LoadWave("SE_Eat", "resources/Eat.mp3");
	audio_->PlayBGM("BGM_GameScene", 0.2f);

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
		hipDropDamageSprites_[i]->SetAnchorPoint({0.5f, 0.5f});
	}

	// ダメージエフェクトスプライト
	for (int i = 0; i < damageEffectSprite_.size(); ++i) {
		damageEffectSprite_[i] = std::make_unique<Sprite>();
		damageEffectSprite_[i]->Initialize(engineContext_, "resources/1.png");
		damageEffectSprite_[i]->SetSize({15.0f, 25.0f});
		damageEffectSprite_[i]->SetColor({0.7f, 0.7f, 0.7f, 1.0f});
		damageEffectSprite_[i]->SetEnableShine(true);
		damageEffectSprite_[i]->SetShineColor({1.0f, 1.0f, 1.0f, 1.0f});
		damageEffectSprite_[i]->SetShineParams({0.0f, 0.5f, 1.0f, 1.0f});
	}

	// プレイヤーのヒップドロップパワースプライト
	hipDropPowerSprite_ = std::make_unique<Sprite>();
	hipDropPowerSprite_->Initialize(engineContext_, "resources/attackAmount.png");
	hipDropPowerSprite_->SetPosition({1140.0f, 70.0f});

	// 砲台の生成&初期化
	gunTurret_ = std::make_unique<GunTurret>();
	gunTurret_->Initialize(engineContext_);

	// プレイヤーの攻撃力背景スプライト
	hipDropPowerBG_ = std::make_unique<HipDropPowerBG>();
	hipDropPowerBG_->Initialize(ctx);

	// タイトルシーンのライト初期設定
	engineContext_->object3dCommon->SetDirectionalLightIntensity(0.5f);

	// 座標変換便利クラス
	screenSpaceUtility_ = std::make_unique<ScreenSpaceUtility>();

	// 操作説明スプライト
	moveSprite_ = std::make_unique<Sprite>();
	moveSprite_->Initialize(ctx, "resources/move.png");
	moveSprite_->SetPosition({10.0f, 5.0f});

	aButtonSprite_ = std::make_unique<Sprite>();
	aButtonSprite_->Initialize(ctx, "resources/AButton.png");
	aButtonSprite_->SetPosition({758.0f, 5.0f});

	// 隕石のジェネレーター
	// meteoriteGenerator_=std::make_unique<MeteoriteGenerator>();
	// meteoriteGenerator_->Initialize(ctx);
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
	CollisionPlayerTurretBullet();

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
		block->Update(timeManager_->GetDeltaTime());
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

	// プレイヤーの攻撃力背景スプライト
	hipDropPowerBG_->Update(timeManager_->GetDeltaTime());

	// ヒップドロップダメージ表示の更新
	UpdateHipDropDamageDisplay();

	// ゲーム終了判定
	EndGameCheck();

	// ヒップドロップパワースプライト更新
	hipDropPowerSprite_->Update();

	// 十字エフェクト
	for (auto& effect : crossEffects_) {
		effect->Update(timeManager_->GetDeltaTime(), engineContext_->object3dCommon->GetDefaultCamera());
	}

	// アニメーションが終わっていれば十字エフェクトを削除
	std::erase_if(crossEffects_, [&](auto& effect) { return !effect->GetIsAnimation(); });

	// オーディオ更新
	audio_->Update();

	// 隕石のジェネレーター
	// meteoriteGenerator_->Update(timeManager_->GetDeltaTime());

	moveSprite_->Update();
	aButtonSprite_->Update();

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

	// 隕石のジェネレーター
	// meteoriteGenerator_->Draw();

	// Particle
	particleDustPlayer_->Draw();
	particleDustEnemy_->Draw();

	// 十字エフェクト
	for (auto& effect : crossEffects_) {
		effect->Draw();
	}

	// 目標フルーツ背景スプライトの描画
	targetFruitBG_->Draw();

	// 目標フルーツの描画
	DrawTargetFruits();

	// 画面両端の幕
	rightCurtain_->Draw();
	leftCurtain_->Draw();

	// プレイヤーの攻撃力背景スプライト
	hipDropPowerBG_->Draw();

	// ヒップドロップダメージの描画
	for (auto& sprite : hipDropDamageSprites_) {
		sprite->Draw();
	}

	// ダメージエフェクトの描画
	for (auto& sprite : damageEffectSprite_) {
		if (animationDamageEffect_.anim.GetIsActive()) {
			sprite->Draw();
		}
	}

	// ヒップドロップパワースプライト描画
	hipDropPowerSprite_->Draw();

	moveSprite_->Draw();
	aButtonSprite_->Draw();
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
	if (audio_) {
		audio_->StopBGM();
	}
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
	ImGuiFPS();
	ImGuiDebugCamera();
	engineContext_->object3dCommon->DrawImGuiLighting();
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
			enemy_->SetIsHitPlayerHipDrop(true);
			player_->SetIsHitEnemyHipDrop(true);
			int damage = player_->GetHipDropPowerLevel();
			enemy_->SubHP(damage);         // ダメージ減算
			player_->ResetHipDropDamage(); // ダメージ判定をリセット
			StartShake(30, 0.5f);          // カメラシェイク

			// ダメージエフェクトのアニメーション初期設定
			for (int i = 0; i < damageEffectSprite_.size(); ++i) {
				animationDamageEffect_.anim = {
				    damageEffectSprite_[i]->GetSize(), {30.0f, 50.0f},
                     1.5f, EaseType::EASEOUTBOUNCE
                };
			}

			// SE再生
			audio_->PlaySE("SE_HitBoss");
		}

		// 敵に接触したらダメージ判定を無効化
		player_->SetIsHitEnemyHipDrop(true); // これにより敵への連続ダメージを防ぐ
	}
}

void GamePlayScene::CollisionPlayerFruits() {
	HandleFruitCollision(grapes_, "grape");
	HandleFruitCollision(apples_, "apple");
	HandleFruitCollision(oranges_, "orange");
}

template<typename FruitType> void GamePlayScene::HandleFruitCollision(std::vector<std::unique_ptr<FruitType>>& fruits, const std::string& fruitName) {
	fruits.erase(
	    std::remove_if(
	        fruits.begin(), fruits.end(),
	        [this, &fruitName](std::unique_ptr<FruitType>& fruit) {
		        if (Collision::Intersect(player_->GetAABB(), fruit->GetAABB())) {
			        // 目標フルーツの中から未収集のフルーツを探す
			        for (auto& targetFruit : fruitOrder_) {
				        if (targetFruit.fruitType == fruitName && !targetFruit.isCollected) {
					        targetFruit.isCollected = true;
					        collectedFruitCount_++;
					        // フルーツの色を変更（グレーアウト）
					        targetFruit.sprite->SetColor({0.5f, 0.5f, 0.5f, 0.5f});
					        break;
				        }
			        }
			        player_->SetIsPowerUp(true);

			        // フルーツ取得時プレイヤーのスケールアニメーションフラグをたてる
			        player_->SetIsGetFruit(true);
			        player_->SetFruitGetAnimation(); // アニメーションの初期設定

			        // 十字エフェクトの生成&初期化
			        auto crossEffect = std::make_unique<CrossEffect>();
			        crossEffect->Initialize(engineContext_, player_->GetTranslate());
			        crossEffects_.push_back(std::move(crossEffect));

			        // SE再生
			        audio_->PlaySE("SE_Eat", 0.4f);

			        return true;
		        }
		        return false;
	        }),
	    fruits.end());
}

void GamePlayScene::CollisionPlayerTurretBullet() {
	gunTurret_->GetBullet().erase(
	    std::remove_if(
	        gunTurret_->GetBullet().begin(), gunTurret_->GetBullet().end(),
	        [this](std::unique_ptr<TurretBullet>& bullet) {
		        if (Collision::Intersect(player_->GetAABB(), bullet->GetAABB())) {
			        player_->SetIsHitEnemy(true);
			        return true;
		        }
		        return false;
	        }),
	    gunTurret_->GetBullet().end());
}

void GamePlayScene::CreateGrape() {
	if (grapes_.size() >= grapeGenerator_.kCount)
		return;

	// 生成タイマーカウント
	grapeGenerator_.generateTimer += timeManager_->GetDeltaTime();

	if (grapeGenerator_.generateTimer >= grapeGenerator_.kGenerateTimer) {
		auto grape = std::make_unique<Grape>();
		grape->Initialize(engineContext_, "grape");

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
		apple->Initialize(engineContext_, "apple");

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
		orange->Initialize(engineContext_, "orange");

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
			cameraTargetPos_ = player_->GetTranslate(); // カメラの目標ポジションを設定
		} else if (enemy_->GetHP() <= 0) {
			resultStatus = "StageClear.obj";
			cameraTargetPos_ = enemy_->GetTranslate(); // カメラの目標ポジションを設定
		}

		// 死亡時カメラアニメーション
		// CameraAnimation();

		// カメラアニメーションが終わったら
		// if (isAnimationEnd_) {
		// 結果文字列をファイルに保存
		SaveResultStatus(resultStatus);
		RequestSceneChange("Result");
		//}
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

	// カメラのセット
	screenSpaceUtility_->SetCamera(debugCamera_);

	// 攻撃力アニメーション初期設定
	if (!animationHipDropPower_.anim.GetIsActive()) {
		animationHipDropPower_.anim = {
		    hipDropDamageSprites_[0]->GetSize(), {40.0f, 60.0f},
             0.5f, EaseType::EASEOUTCUBIC
        };
	} else {
		animationHipDropPower_.anim.Update(timeManager_->GetDeltaTime(), animationHipDropPower_.temp);
		for (auto& sprite : hipDropDamageSprites_) {
			sprite->SetSize(animationHipDropPower_.temp); // サイズの適用
		}
	}

	// ダメージエフェクトアニメーション
	if (animationDamageEffect_.anim.GetIsActive()) {
		animationDamageEffect_.anim.Update(timeManager_->GetDeltaTime(), animationDamageEffect_.temp);
	}

	if (digit10 > 0) { // 10の位がある場合のみ表示
		// 画面右上の攻撃力HUD
		hipDropDamageSprites_[0]->SetTexture("resources/" + std::to_string(digit10) + ".png");
		hipDropDamageSprites_[0]->SetPosition({1170.0f, 140.0f}); // 位置調整
		hipDropDamageSprites_[0]->Update();

		// ダメージエフェクト
		damageEffectSprite_[0]->SetTexture("resources/" + std::to_string(digit10) + ".png");
		damageEffectSprite_[0]->SetSize(animationDamageEffect_.temp);
		damageEffectSprite_[0]->SetPosition(screenSpaceUtility_->WorldToScreen(enemy_->GetTranslate(), {2.0f, 2.0f}));
		damageEffectSprite_[0]->Update();
	} else {
		hipDropDamageSprites_[0]->SetSize({0.0f, 0.0f}); // 10の位が0の場合は表示しない
		damageEffectSprite_[0]->SetSize({0.0f, 0.0f});   // 10の位が0の場合は表示しない
	}

	// 画面右上の攻撃力HUD
	hipDropDamageSprites_[1]->SetTexture("resources/" + std::to_string(digit1) + ".png");
	hipDropDamageSprites_[1]->SetPosition({1210.0f, 140.0f}); // 位置調整
	hipDropDamageSprites_[1]->Update();

	// ダメージエフェクト
	damageEffectSprite_[1]->SetTexture("resources/" + std::to_string(digit1) + ".png");
	damageEffectSprite_[1]->SetSize(animationDamageEffect_.temp);
	damageEffectSprite_[1]->SetPosition(screenSpaceUtility_->WorldToScreen(enemy_->GetTranslate(), {4.0f, 2.0f}));
	damageEffectSprite_[1]->Update();
}

void GamePlayScene::CameraAnimation() {
	if (isAnimationEnd_) // アニメーションがすでに終わっていたら早期リターン
		return;

	if (!cameraAnimation_.anim.GetIsActive()) {
		// ターゲットとカメラの中間地点
		Vector3 midPos = debugCamera_->GetTranslation() + (cameraTargetPos_ - debugCamera_->GetTranslation()) * 0.8f;

		// カメラアニメーションの初期設定
		cameraAnimation_.anim = {debugCamera_->GetTranslation(), midPos, 1.0f, EaseType::EASEINOUTBACK};
	} else {
		bool playing = cameraAnimation_.anim.Update(timeManager_->GetDeltaTime(), cameraAnimation_.temp);
		debugCamera_->SetTranslation(cameraAnimation_.temp);

		if (!playing) {
			// アニメーションの終了フラグを立てる
			isAnimationEnd_ = true;
		}
	}
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
		newSprite->SetEnableShine(true);
		newSprite->SetShineColor({0.8f, 0.8f, 0.8f, 1.0f});
		newSprite->SetShineParams({0.0f, 0.3f, 0.5f, 1.0f});
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
			sprite->SetEnableShine(false);              // 発光を切る
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