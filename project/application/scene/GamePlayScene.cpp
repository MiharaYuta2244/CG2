#include "GamePlayScene.h"
#include "Collision.h"
#include "HitFlashModule.h"
#include "HitRingModule.h"
#include "HitSparkModule.h"
#include "SceneManager.h"
#include <algorithm>

using namespace TinyEngine;

void GamePlayScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();

	// 共通データをリセット
	commonData_->killCount = 0;
	commonData_->clearTime = 0.0f;

	// デバッグカメラ
	debugCamera_ = std::make_unique<Camera>();
	debugCamera_->Initialize();

	// プレイヤーの生成
	player_ = std::make_unique<Player>();

	// メインカメラ
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize();
	mainCamera_->SetTranslation({0.0f, 60.0f, 0.0f});
	mainCamera_->SetPivot(player_->GetPosition());
	mainCamera_->SetEuler({std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f});
	currentCameraPivot_ = player_->GetPosition();
	cameraPosY_ = -14.0f;

	// カメラの設定
	ctx_.currentCamera = mainCamera_.get();
	ctx_.engineContext->object3dCommon->SetDefaultCamera(ctx_.currentCamera);
	ctx_.engineContext->particleCommon->SetDefaultCamera(ctx_.currentCamera);

	// 血痕管理インスタンス
	bloodDecalManager_ = std::make_unique<BloodDecalManager>();
	bloodDecalManager_->Initialize(ctx.engineContext, "Dust.png");

	// プレイヤーの初期化
	player_->Initialize(ctx_.engineContext, bloodDecalManager_.get());

	// 敵の生成&初期化
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize(ctx_.engineContext, bloodDecalManager_.get());

	// 敵の弾管理インスタンス生成
	enemyBulletManager_ = std::make_unique<EnemyBulletManager>();

	// 壁の管理インスタンス生成&初期化
	wallManager_ = std::make_unique<WallManager>();
	wallManager_->Initialize(ctx_.engineContext);

	// ドアの管理インスタンス生成&初期化
	doorManager_ = std::make_unique<DoorManager>();
	doorManager_->Initialize(ctx_.engineContext);

	// ガラスの管理インスタンス生成&初期化
	glassManager_ = std::make_unique<GlassManager>();
	glassManager_->Initialize(ctx_.engineContext);

	// ゴール判定インスタンス生成&初期化
	goal_ = std::make_unique<Goal>();
	goal_->Initialize(ctx_.engineContext);

	// フラッシュエフェクトの生成&初期化
	flashEffect_ = std::make_unique<FlashEffect>();
	flashEffect_->Initialize(ctx_.engineContext);

	// レターボックスの生成&初期化
	letterBox_ = std::make_unique<LetterBox>();
	letterBox_->Initialize(ctx_.engineContext);

	// 地面の生成&初期化
	ground_ = std::make_unique<Ground>();
	ground_->Initialize(ctx_.engineContext);

	// 檻
	cageManager_ = std::make_unique<CageManager>();
	cageManager_->Initialize(ctx.engineContext);

	// シーン遷移要求制御変数
	isTransitionRequested_ = false;

	// 操作方法UI
	controls_ = std::make_unique<Controls>();
	controls_->Initialize(ctx_.engineContext);

	// シーンで使うエフェクトの宣言
	ctx_.engineContext->postEffectPipeline->SetEffects({
	    PostEffectType::Vignette,    // ビネット
	    PostEffectType::Glitch,      // グリッチ
	    PostEffectType::DeathEffect, // 死亡時エフェクト
	});

	// パラメータ設定
	auto* vignette = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Vignette);
	if (vignette) {
		vignette->SetVignetteColor({1, 0, 0, 1});
	}

	// RadialBlurのアニメーション開始フラグ
	isDeathAnimStarted_ = false;

	// プレイヤー死亡時カメラ演出用インスタンス生成
	cameraZoomController_ = std::make_unique<CameraDeathZoomController>();

	// ギズモで操作する用にリストに追加
	objects_.push_back(player_.get()); // プレイヤー
	objects_.push_back(goal_.get());   // ゴール
	for (auto& enemy : enemyManager_->GetEnemies()) {
		objects_.push_back(enemy.get()); // 敵
	}
	for (auto& wall : wallManager_->GetWalls()) {
		objects_.push_back(wall.get()); // 壁
	}
	for (auto& door : doorManager_->GetDoors()) {
		objects_.push_back(door.get()); // ドア
	}
	for (auto& cage : cageManager_->GetCages()) {
		objects_.push_back(cage.get()); // 檻
	}
}

void GamePlayScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

	// グリッチノイズの更新
	UpdateGlitch(deltaTime);

	// ポーズ画面
	if (ctx_.keyboard->KeyTriggered(DIK_TAB) || ctx_.gamePad->GetState().buttonsPressed.start) {
		RequestScenePush("Pause");
		return;
	}

	// ゴールしていなければクリアタイムを加算
	if (!goal_->GetGoal() && !player_->IsDead()) {
		commonData_->clearTime += deltaTime;
	}

	// Projectionの逆行列をCopyImageに渡す
	Matrix4x4 projInv = MathUtility::Inverse(ctx_.currentCamera->GetProjection());
	auto* outline = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::DepthOutline);
	if (outline) {
		outline->SetProjectionInverse(projInv);
	}

	// HP量に応じてVignetteのパラメータを変更
	auto* vignette = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Vignette);
	if (!player_->IsDead()) {
		if (vignette) {
			float intensity = player_->GetCurrentHP() <= 1.0f ? 0.5f : 0.0f;
			vignette->SetVignetteIntensity(intensity);
		}
	} else {
		vignette->SetVignetteIntensity(0.0f);
	}

	// プレイヤーの更新処理
	player_->Update(deltaTime, ctx_.keyboard, ctx_.gamePad, enemyManager_.get());

	// 敵の更新処理
	enemyManager_->Update(deltaTime, player_.get(), enemyBulletManager_.get(), wallManager_.get(), doorManager_.get(), glassManager_.get());

	// 敵の弾の更新処理
	enemyBulletManager_->Update(deltaTime);

	// 壁の管理インスタンス更新
	wallManager_->Update(deltaTime);

	// ドアの管理インスタンス更新
	doorManager_->Update(deltaTime, player_->GetPosition());

	// ガラスの管理インスタンス更新
	glassManager_->Update();

	// 檻の管理インスタンス
	cageManager_->Update(deltaTime);

	// ゴール判定インスタンス更新
	goal_->Update(deltaTime);

	// 当たり判定
	CollisionGameObjects();

	// 押し戻し完了後の最終的な座標で、描画更新&AABB更新
	player_->PostUpdate();
	enemyManager_->PostUpdate();

	// ゴールしていたらフラッシュ開始
	if (goal_->GetGoal()) {
		flashEffect_->Trigger();
	}

	bool isPlaying = numSamplesAnim_.anim.Update(deltaTime, numSamplesAnim_.temp);

	// フラッシュの演出が終わったらレターボックスの出現
	if (flashEffect_->Finish()) {
		letterBox_->Trigger();

		// 死亡演出の開始処理
		if (!isDeathAnimStarted_) {
			isDeathAnimStarted_ = true;
			numSamplesAnim_.anim.Start(0.0f, 1.0f, 1.0f, EaseType::EASEOUTCIRC);
		}
	}

	// レターボックスの演出が終わったらシーン遷移
	if (!letterBox_->GetIsActive() && !isPlaying) {
		if (!isTransitionRequested_) {
			RequestSceneChange("Result");
			isTransitionRequested_ = true;
		}
	}

	// プレイヤーの生死状態に応じてDeathEffectの強度を制御
	auto* deathEffect = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::DeathEffect);
	if (deathEffect) {
		if (player_->IsDead()) {
			// プレイヤー死亡時はエフェクトを全開にする
			deathEffect->SetDeathEffectIntensity(1.0f);
		} else {
			// 生存時は通常画面
			deathEffect->SetDeathEffectIntensity(0.0f);
		}
	}

	// プレイヤーが死亡したらシーン遷移
	if (player_->IsDead()) {
		// カメラ演出開始
		cameraZoomController_->Start(ctx_.currentCamera->GetTranslation().y);

		// カメラ演出更新
		Vector3 currentPos = ctx_.currentCamera->GetTranslation();
		Vector3 nextPos = {currentPos.x, cameraZoomController_->Update(deltaTime), currentPos.z};
		ctx_.currentCamera->SetTranslation(nextPos);

		if (!isTransitionRequested_ && cameraZoomController_->GetIsFinished()) {
			RequestSceneChange("Result");
			isTransitionRequested_ = true;
		}
	}

	// 壁の管理インスタンスImGui
	wallManager_->DrawImGui();

	// ドアの管理インスタンスImGui
	doorManager_->DrawImGui();

	// ガラスの管理インスタンスImGui
	glassManager_->DrawImGui();

	// 敵の管理インスタンスImGui
	enemyManager_->DrawImGui();

	// 檻の管理インスタンスImGui
	cageManager_->DrawImGui();

	// マウスの画面座標をRayに変換して判定を取る処理
	UpdatePicking();

	// ギズモ用ImGui更新
	UpdateImGui();

	// デバッグ用のImGui更新
	UpdateDebugImGui();

	// パーティクルの更新
	for (auto& particle : enemyDeathParticle_) {
		particle->Update();
	}
	std::erase_if(enemyDeathParticle_, [this](const std::unique_ptr<TinyEngine::Particle>& p) {
		ctx_.currentCamera->StartShake(0.2f, 0.2f);
		return p->IsFinished();
	});

	// カメラの追従
	if (!isDebugCameraActive_) {
		Vector3 playerPos = player_->GetPosition();
		Vector3 playerRot = player_->GetRotation();

		// プレイヤーの向いている方向ベクトルを計算
		Vector3 forward = {std::sin(playerRot.y), 0.0f, std::cos(playerRot.y)};

		// 目標のピボット位置
		Vector3 targetPivot = {playerPos.x + forward.x * offsetDistance_, playerPos.y, playerPos.z + forward.z * offsetDistance_};

		// 線形補間を使ってカメラを滑らかに追従させる
		float followSpeed = 5.0f; // 追従の滑らかさ
		currentCameraPivot_.x += (targetPivot.x - currentCameraPivot_.x) * followSpeed * deltaTime;
		currentCameraPivot_.y = cameraPosY_;
		currentCameraPivot_.z += (targetPivot.z - currentCameraPivot_.z) * followSpeed * deltaTime;

		ctx_.currentCamera->SetPivot(currentCameraPivot_);
	}

	// カメラのシェイク更新
	ctx_.currentCamera->ShakeCamera(deltaTime, shakePower_);

	// カメラの更新
	ctx_.currentCamera->Update(*ctx_.keyboard, *ctx_.gamePad);

	// フラッシュエフェクト更新
	flashEffect_->Update(deltaTime);

	// レターボックス更新
	letterBox_->Update(deltaTime);

	// 地面の更新
	ground_->Update();

	// 操作方法UI更新
	controls_->Update();

	// 血痕管理インスタンス更新
	bloodDecalManager_->SetCamera(ctx_.currentCamera);
	bloodDecalManager_->Update();

	// 入力系のデバッグ処理
	DebugInput();
}

void GamePlayScene::Draw() {
	// 不透明オブジェクトの描画準備
	ctx_.engineContext->object3dCommon->DrawSettingCommon(ctx_.engineContext->textureManager);

	// 地面の描画
	ground_->Draw();

	// ドアの管理インスタンス描画
	doorManager_->Draw();

	// 壁の管理インスタンス描画
	wallManager_->Draw();

	// 敵の描画処理
	enemyManager_->Draw();

	// 檻の管理インスタンス描画
	cageManager_->Draw();

	// 敵の弾の描画処理
	enemyBulletManager_->Draw();

	// プレイヤーの描画処理
	player_->Draw();

	// パーティクルの描画
	for (auto& particle : enemyDeathParticle_) {
		particle->Draw();
	}

	// 血痕描画
	bloodDecalManager_->Draw();

	// ゴール判定インスタンス描画
	goal_->Draw();

	// フラッシュエフェクト描画
	flashEffect_->Draw();

	// レターボックス描画
	letterBox_->Draw();

	// 操作方法UI描画
	controls_->Draw();

	// 半透明オブジェクトの描画準備
	ctx_.engineContext->object3dCommon->DrawSettingTransparent(ctx_.engineContext->textureManager);

	// ガラスの管理インスタンス描画
	glassManager_->Draw();
}

void GamePlayScene::Finalize() {
	// シーン終了時にエフェクトをデフォルトに戻す
	ctx_.engineContext->postEffectPipeline->SetEffects({PostEffectType::FullScreen});
}

void GamePlayScene::CollisionGameObjects() {
	// ==========================================
	// プレイヤーと敵の弾の当たり判定
	// ==========================================
	AABB playerCol = player_->GetBodyCol();
	for (auto& bullet : enemyBulletManager_->GetBullets()) {
		// 発射された1フレーム目以外は判定を行わない
		if (!bullet->IsCollisionActive()) {
			continue;
		}

		// 弾が有効で、かつプレイヤーと接触しているか
		if (Collision::Intersect(player_->GetBodyCol(), bullet->GetCollision())) {

			bool isGuarded = false; // 弾を防げたかどうかのフラグ

			if (!ctx_.currentCamera->GetIsShake()) {
				// カメラシェイク
				ctx_.currentCamera->StartShake(0.1f, 0.1f);
			}

			// プレイヤーが敵を掴んでいるかチェック
			if (player_->IsGrabbingEnemy()) {

				// プレイヤーの向きベクトルを取得
				Vector2 playerDir = player_->GetDirection();

				// プレイヤーから弾への方向ベクトルを計算し、正規化する
				Vector3 playerToBullet = bullet->GetPosition() - player_->GetPosition();
				playerToBullet = MathUtility::Normalize(playerToBullet);

				// 内積を計算して、正面から来ているか判定する
				float dot = playerDir.x * playerToBullet.x + playerDir.y * playerToBullet.z;

				if (dot < 0.0f) {
					isGuarded = true;
					Enemy* shieldEnemy = player_->GetGrabbedEnemy();
					if (shieldEnemy) {
						shieldEnemy->Damage();
						if (shieldEnemy->IsDead()) {
							commonData_->killCount += 1;
							GenerateEnemyDeathParticle(shieldEnemy->GetPos());
						}
					}
				}
			}

			if (!isGuarded) {
				// 盾で防げなかった
				player_->Damage(1);
			}
		}
	}

	// ==========================================
	// プレイヤーの攻撃用範囲と敵の当たり判定
	// ==========================================
	for (auto& enemy : enemyManager_->GetEnemies()) {
		if (Collision::Intersect(player_->GetAttackCol(), enemy->GetBodyCol())) {
			// プレイヤーの攻撃フラグを立てる
			player_->SetEnableAttack(true);
		}
	}

	// ==========================================
	// プレイヤーの攻撃用範囲とガラスの当たり判定
	// ==========================================
	if (player_->GetIsAttack()) {
		for (auto& glass : glassManager_->GetGlasses()) {
			if (Collision::Intersect(player_->GetAttackCol(), glass->GetCollision())) {
				// ガラス削除
				glassManager_->RemoveGlass(glass.get());
				break;
			}
		}
	}

	// ==========================================
	// プレイヤーと壁の押し出し判定
	// ==========================================
	AABB playerAABB = player_->GetBodyCol();
	Vector3 playerPos = player_->GetPosition();

	for (const auto& wall : wallManager_->GetWalls()) {
		AABB wallAABB = wall->GetCollision();

		// AABB同士の交差判定
		if (playerAABB.min.x <= wallAABB.max.x && playerAABB.max.x >= wallAABB.min.x && playerAABB.min.y <= wallAABB.max.y && playerAABB.max.y >= wallAABB.min.y &&
		    playerAABB.min.z <= wallAABB.max.z && playerAABB.max.z >= wallAABB.min.z) {

			// めり込み量の計算
			// X軸方向のめり込み量
			float overlapX1 = wallAABB.max.x - playerAABB.min.x; // 右から左へ押す量
			float overlapX2 = playerAABB.max.x - wallAABB.min.x; // 左から右へ押す量
			// Z軸方向のめり込み量
			float overlapZ1 = wallAABB.max.z - playerAABB.min.z; // 奥から手前へ押す量
			float overlapZ2 = playerAABB.max.z - wallAABB.min.z; // 手前から奥へ押す量

			// 最小のめり込み量を選ぶ（正の値にする）
			float minOverlapX = std::min(overlapX1, overlapX2);
			float minOverlapZ = std::min(overlapZ1, overlapZ2);

			// めり込みが少ない軸の方向に押し出す
			if (minOverlapX < minOverlapZ) {
				// X軸方向に押し出す
				if (overlapX1 < overlapX2) {
					playerPos.x += overlapX1; // 右へ押し出す
				} else {
					playerPos.x -= overlapX2; // 左へ押し出す
				}
			} else {
				// Z軸方向に押し出す
				if (overlapZ1 < overlapZ2) {
					playerPos.z += overlapZ1; // 奥へ押し出す
				} else {
					playerPos.z -= overlapZ2; // 手前へ押し出す
				}
			}

			// 押し出した結果をプレイヤーの座標に反映
			player_->SetPosition(playerPos);

			// 複数の壁と連続で当たるケースを考慮し、判定用AABBもその場で更新
			playerAABB.max.x = playerPos.x + 0.5f;
			playerAABB.min.x = playerPos.x - 0.5f;
			playerAABB.max.z = playerPos.z + 0.5f;
			playerAABB.min.z = playerPos.z - 0.5f;
		}
	}

	// ==========================================
	// プレイヤーとガラスの押し出し判定
	// ==========================================
	for (const auto& glass : glassManager_->GetGlasses()) {
		AABB glassAABB = glass->GetCollision();

		// AABB同士の交差判定
		if (playerAABB.min.x <= glassAABB.max.x && playerAABB.max.x >= glassAABB.min.x && playerAABB.min.y <= glassAABB.max.y && playerAABB.max.y >= glassAABB.min.y &&
		    playerAABB.min.z <= glassAABB.max.z && playerAABB.max.z >= glassAABB.min.z) {

			// めり込み量の計算
			// X軸方向のめり込み量
			float overlapX1 = glassAABB.max.x - playerAABB.min.x; // 右から左へ押す量
			float overlapX2 = playerAABB.max.x - glassAABB.min.x; // 左から右へ押す量
			// Z軸方向のめり込み量
			float overlapZ1 = glassAABB.max.z - playerAABB.min.z; // 奥から手前へ押す量
			float overlapZ2 = playerAABB.max.z - glassAABB.min.z; // 手前から奥へ押す量

			// 最小のめり込み量を選ぶ（正の値にする）
			float minOverlapX = std::min(overlapX1, overlapX2);
			float minOverlapZ = std::min(overlapZ1, overlapZ2);

			// めり込みが少ない軸の方向に押し出す
			if (minOverlapX < minOverlapZ) {
				// X軸方向に押し出す
				if (overlapX1 < overlapX2) {
					playerPos.x += overlapX1; // 右へ押し出す
				} else {
					playerPos.x -= overlapX2; // 左へ押し出す
				}
			} else {
				// Z軸方向に押し出す
				if (overlapZ1 < overlapZ2) {
					playerPos.z += overlapZ1; // 奥へ押し出す
				} else {
					playerPos.z -= overlapZ2; // 手前へ押し出す
				}
			}

			// 押し出した結果をプレイヤーの座標に反映
			player_->SetPosition(playerPos);

			// 複数の壁と連続で当たるケースを考慮し、判定用AABBもその場で更新
			playerAABB.max.x = playerPos.x + 0.5f;
			playerAABB.min.x = playerPos.x - 0.5f;
			playerAABB.max.z = playerPos.z + 0.5f;
			playerAABB.min.z = playerPos.z - 0.5f;
		}
	}

	// ==========================================
	// プレイヤーと檻の押し出し判定
	// ==========================================
	for (const auto& cage : cageManager_->GetCages()) {
		AABB cageAABB = cage->GetCollision();

		// AABB同士の交差判定
		if (playerAABB.min.x <= cageAABB.max.x && playerAABB.max.x >= cageAABB.min.x && playerAABB.min.y <= cageAABB.max.y && playerAABB.max.y >= cageAABB.min.y &&
		    playerAABB.min.z <= cageAABB.max.z && playerAABB.max.z >= cageAABB.min.z) {

			// めり込み量の計算
			// X軸方向のめり込み量
			float overlapX1 = cageAABB.max.x - playerAABB.min.x; // 右から左へ押す量
			float overlapX2 = playerAABB.max.x - cageAABB.min.x; // 左から右へ押す量
			// Z軸方向のめり込み量
			float overlapZ1 = cageAABB.max.z - playerAABB.min.z; // 奥から手前へ押す量
			float overlapZ2 = playerAABB.max.z - cageAABB.min.z; // 手前から奥へ押す量

			// 最小のめり込み量を選ぶ（正の値にする）
			float minOverlapX = std::min(overlapX1, overlapX2);
			float minOverlapZ = std::min(overlapZ1, overlapZ2);

			// めり込みが少ない軸の方向に押し出す
			if (minOverlapX < minOverlapZ) {
				// X軸方向に押し出す
				if (overlapX1 < overlapX2) {
					playerPos.x += overlapX1; // 右へ押し出す
				} else {
					playerPos.x -= overlapX2; // 左へ押し出す
				}
			} else {
				// Z軸方向に押し出す
				if (overlapZ1 < overlapZ2) {
					playerPos.z += overlapZ1; // 奥へ押し出す
				} else {
					playerPos.z -= overlapZ2; // 手前へ押し出す
				}
			}

			// 押し出した結果をプレイヤーの座標に反映
			player_->SetPosition(playerPos);

			// 複数の壁と連続で当たるケースを考慮し、判定用AABBもその場で更新
			playerAABB.max.x = playerPos.x + 0.5f;
			playerAABB.min.x = playerPos.x - 0.5f;
			playerAABB.max.z = playerPos.z + 0.5f;
			playerAABB.min.z = playerPos.z - 0.5f;
		}
	}

	// ==========================================
	// 敵と壁の判定（めり込み防止 ＆ ノックバック時死亡）
	// ==========================================
	for (auto& enemy : enemyManager_->GetEnemies()) {
		if (enemy->IsDead())
			continue;

		AABB enemyAABB = enemy->GetBodyCol();
		Vector3 enemyPos = enemy->GetPos();

		for (const auto& wall : wallManager_->GetWalls()) {
			AABB wallAABB = wall->GetCollision();

			// AABB同士の交差判定
			if (enemyAABB.min.x <= wallAABB.max.x && enemyAABB.max.x >= wallAABB.min.x && enemyAABB.min.y <= wallAABB.max.y && enemyAABB.max.y >= wallAABB.min.y && enemyAABB.min.z <= wallAABB.max.z &&
			    enemyAABB.max.z >= wallAABB.min.z) {

				// ----------------------------------------
				// 壁激突死の判定：もしノックバック中なら死亡させて処理を抜ける
				// ----------------------------------------
				if (enemy->IsKnockBack()) {
					enemy->Kill();
					commonData_->killCount++;

					// パーティクルの生成
					GenerateEnemyDeathParticle(enemy->GetPos());

					break;
				}

				// ----------------------------------------
				// 通常時の押し出し処理
				// ----------------------------------------
				float overlapX1 = wallAABB.max.x - enemyAABB.min.x;
				float overlapX2 = enemyAABB.max.x - wallAABB.min.x;
				float overlapZ1 = wallAABB.max.z - enemyAABB.min.z;
				float overlapZ2 = enemyAABB.max.z - wallAABB.min.z;

				float minOverlapX = std::min(overlapX1, overlapX2);
				float minOverlapZ = std::min(overlapZ1, overlapZ2);

				if (minOverlapX < minOverlapZ) {
					if (overlapX1 < overlapX2) {
						enemyPos.x += overlapX1;
					} else {
						enemyPos.x -= overlapX2;
					}
				} else {
					if (overlapZ1 < overlapZ2) {
						enemyPos.z += overlapZ1;
					} else {
						enemyPos.z -= overlapZ2;
					}
				}

				// 結果を敵の座標に反映
				enemy->SetPos(enemyPos);

				// 連続で壁に当たるケースを考慮してAABB更新
				enemyAABB.max.x = enemyPos.x + 0.5f;
				enemyAABB.min.x = enemyPos.x - 0.5f;
				enemyAABB.max.z = enemyPos.z + 0.5f;
				enemyAABB.min.z = enemyPos.z - 0.5f;
			}
		}
	}

	// =========================================================
	// 敵とドアの当たり判定
	// =========================================================
	for (auto& enemy : enemyManager_->GetEnemies()) {
		for (auto& door : doorManager_->GetDoors()) {
			AABB wallAABB = door->GetCollision();
			AABB enemyAABB = enemy->GetBodyCol();
			Vector3 enemyPos = enemy->GetPos();

			// ドアが開いているなら敵は通り抜けられるのでスキップ
			if (door->GetIsOpen()) {
				continue;
			}

			// 衝突判定
			if (Collision::Intersect(enemy->GetBodyCol(), door->GetCollision())) {
				// 押し戻し・衝突応答処理
				// ----------------------------------------
				// 壁激突死の判定：もしノックバック中なら死亡させて処理を抜ける
				// ----------------------------------------
				if (enemy->IsKnockBack()) {
					enemy->Kill();
					commonData_->killCount++;

					// パーティクルの生成
					GenerateEnemyDeathParticle(enemy->GetPos());

					break;
				}

				// ----------------------------------------
				// 通常時の押し出し処理
				// ----------------------------------------
				float overlapX1 = wallAABB.max.x - enemyAABB.min.x;
				float overlapX2 = enemyAABB.max.x - wallAABB.min.x;
				float overlapZ1 = wallAABB.max.z - enemyAABB.min.z;
				float overlapZ2 = enemyAABB.max.z - wallAABB.min.z;

				float minOverlapX = std::min(overlapX1, overlapX2);
				float minOverlapZ = std::min(overlapZ1, overlapZ2);

				if (minOverlapX < minOverlapZ) {
					if (overlapX1 < overlapX2) {
						enemyPos.x += overlapX1;
					} else {
						enemyPos.x -= overlapX2;
					}
				} else {
					if (overlapZ1 < overlapZ2) {
						enemyPos.z += overlapZ1;
					} else {
						enemyPos.z -= overlapZ2;
					}
				}

				// 結果を敵の座標に反映
				enemy->SetPos(enemyPos);

				// 連続で壁に当たるケースを考慮してAABB更新
				enemyAABB.max.x = enemyPos.x + 0.5f;
				enemyAABB.min.x = enemyPos.x - 0.5f;
				enemyAABB.max.z = enemyPos.z + 0.5f;
				enemyAABB.min.z = enemyPos.z - 0.5f;
			}
		}
	}

	// ==========================================
	// プレイヤーとゴールの当たり判定
	// ==========================================
	if (Collision::Intersect(player_->GetBodyCol(), goal_->GetCol())) {
		// ゴールフラグを立てる
		goal_->SetGoal(true);
	}

	// ==========================================
	// 敵同士の当たり判定
	// ==========================================
	auto& enemies = enemyManager_->GetEnemies();

	for (auto itA = enemies.begin(); itA != enemies.end(); ++itA) {
		Enemy* a = itA->get();
		if (a->IsDead())
			continue;

		auto itB = itA;
		++itB;

		for (; itB != enemies.end(); ++itB) {
			Enemy* b = itB->get();
			if (b->IsDead())
				continue;

			// 当たり判定
			if (Collision::Intersect(a->GetBodyCol(), b->GetBodyCol())) {
				// どちらかがノックバック状態であれば
				if (a->IsKnockBack() || b->IsKnockBack()) {
					a->Damage();
					b->Damage();
					a->StopAnimation();
					b->StopAnimation();

					// ダメージの結果死亡した場合のみ処理を行う
					if (a->IsDead()) {
						commonData_->killCount += 1;
						GenerateEnemyDeathParticle(a->GetPos());
					}
					if (b->IsDead()) {
						commonData_->killCount += 1;
						GenerateEnemyDeathParticle(b->GetPos());
					}
				}
			}
		}
	}

	// ==========================================
	// 敵と敵の弾の当たり判定
	// ==========================================
	if (player_->GetGrabbedEnemy() != nullptr) {
		for (const auto& enemy : enemyManager_->GetEnemies()) {
			for (const auto& bullet : enemyBulletManager_->GetBullets()) {
				// 発射された1フレーム目以外は判定を行わない
				if (!bullet->IsCollisionActive()) {
					continue;
				}

				// 衝突判定
				if (Collision::Intersect(enemy->GetBodyCol(), bullet->GetCollision())) {
					enemy->Damage();

					if (enemy->IsDead()) {
						commonData_->killCount += 1;
						GenerateEnemyDeathParticle(enemy->GetPos());
					}
				}
			}
		}
	}
}

void GamePlayScene::UpdateImGui() {
#ifdef USE_IMGUI
	// 毎フレーム、現在生存しているオブジェクトでリストを再構築
	objects_.clear();
	objects_.push_back(player_.get()); // プレイヤー
	objects_.push_back(goal_.get());   // ゴール
	for (auto& enemy : enemyManager_->GetEnemies()) {
		objects_.push_back(enemy.get()); // 敵
	}
	for (auto& wall : wallManager_->GetWalls()) {
		objects_.push_back(wall.get()); // 壁
	}
	for (auto& door : doorManager_->GetDoors()) {
		objects_.push_back(door.get()); // ドア
	}
	for (auto& glass : glassManager_->GetGlasses()) {
		objects_.push_back(glass.get()); // ガラス
	}
	for (auto& cage : cageManager_->GetCages()) {
		objects_.push_back(cage.get()); // 檻
	}

	// 選択中のオブジェクトが破棄されていないか検証
	bool isSelectedValid = false;
	for (IGameObject* obj : objects_) {
		if (obj == selectedGameObject_) {
			isSelectedValid = true;
			break;
		}
	}

	// 存在しなければ選択を解除
	if (!isSelectedValid) {
		selectedGameObject_ = nullptr;
	}

	ImGui::Begin("Object Manager");

	// 操作モードの選択
	if (ImGui::RadioButton("Translate", currentGizmoOperation_ == ImGuizmo::TRANSLATE)) {
		currentGizmoOperation_ = ImGuizmo::TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", currentGizmoOperation_ == ImGuizmo::ROTATE)) {
		currentGizmoOperation_ = ImGuizmo::ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", currentGizmoOperation_ == ImGuizmo::SCALE)) {
		currentGizmoOperation_ = ImGuizmo::SCALE;
	}

	ImGui::Separator();

	// オブジェクトリストの表示
	ImGui::Text("Objects");
	ImGui::BeginChild("ObjectList", ImVec2(0, 150), true);
	for (size_t i = 0; i < objects_.size(); ++i) {
		IGameObject* obj = objects_[i];
		std::string label = obj->GetName() + " ##" + std::to_string(i);

		bool isSelected = (selectedGameObject_ == obj);
		if (ImGui::Selectable(label.c_str(), isSelected)) {
			selectedGameObject_ = obj;
		}
	}
	ImGui::EndChild();
	ImGui::End();

	// ギズモ描画レイヤー
	if (selectedGameObject_ != nullptr) {
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::Begin("GizmoLayer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar);

		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::Enable(true);
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

		// 対象のTransformを取得
		Transform& transform = selectedGameObject_->GetTransform();

		// ラジアンから度に変換して配列に格納
		float degRot[3] = {transform.rotate.x * 180.0f / std::numbers::pi_v<float>, transform.rotate.y * 180.0f / std::numbers::pi_v<float>, transform.rotate.z * 180.0f / std::numbers::pi_v<float>};

		// Transformの要素から4x4行列を作成
		float objectMatrix[16];
		ImGuizmo::RecomposeMatrixFromComponents(&transform.translate.x, degRot, &transform.scale.x, objectMatrix);
		Matrix4x4 viewMat = ctx_.currentCamera->GetViewMatrix();
		Matrix4x4 projMat = ctx_.currentCamera->GetProjection();

		// ImGuizmoで操作
		ImGuizmo::Manipulate(&viewMat.m[0][0], &projMat.m[0][0], currentGizmoOperation_, currentGizmoMode_, objectMatrix);

		// ギズモが操作中なら、行列から要素を分解してTransformに戻す
		if (ImGuizmo::IsUsing()) {
			float newTrans[3], newRot[3], newScale[3];
			ImGuizmo::DecomposeMatrixToComponents(objectMatrix, newTrans, newRot, newScale);

			// ギズモ操作前の元の高さを保存しておく
			float oldY = transform.translate.y;

			transform.translate = {newTrans[0], newTrans[1], newTrans[2]};

			// Y座標だけは元の高さを維持するように上書き固定する
			transform.translate.y = oldY;

			transform.scale = {newScale[0], newScale[1], newScale[2]};

			// 度からラジアンに戻して保存
			transform.rotate = {newRot[0] * std::numbers::pi_v<float> / 180.0f, newRot[1] * std::numbers::pi_v<float> / 180.0f, newRot[2] * std::numbers::pi_v<float> / 180.0f};
		}

		ImGui::End();
		ImGui::PopStyleColor();
	}
#endif // USE_IMGUI
}

void GamePlayScene::UpdateDebugImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Debug List");
	if (ImGui::Button("Kill Player")) {
		player_->Damage(player_->GetMaxHP());
	}

	ImGui::DragFloat("CameraPosY", &cameraPosY_, 0.1f);
	ImGui::DragFloat3("Direction", &ctx_.engineContext->object3dCommon->GetDirectionalLight().direction.x, 0.01f);
	ImGui::DragFloat3("Color", &ctx_.engineContext->object3dCommon->GetDirectionalLight().color.x, 0.01f);
	ImGui::DragFloat3("Intensity", &ctx_.engineContext->object3dCommon->GetDirectionalLight().intensity, 0.01f);
	ImGui::End();
#endif
}

void GamePlayScene::UpdateGlitch(float deltaTime) {
	// グリッチノイズ用タイマーの減算
	if (glitchTimer_ > 0.0f) {
		glitchTimer_ -= deltaTime;
		elapsedTime_ += deltaTime;
		if (glitchTimer_ < 0.0f) {
			glitchTimer_ = 0.0f;
			elapsedTime_ = 0.0f;
		}
	}

	float intensity = 0.0f;
	if (glitchTimer_ > 0.0f) {
		intensity = 1.0f;
	}

	auto* glitch = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Glitch);
	if (glitch) {
		glitch->SetGlitchTime(elapsedTime_);
		glitch->SetGlitchIntensity(intensity);
	}
}

void GamePlayScene::GenerateEnemyDeathParticle(const Vector3& pos) {
	// 火花
	auto sparks = std::make_unique<Particle>();
	sparks->Initialize(ctx_.engineContext, pos, "white.png", std::make_unique<HitSparkModule>(), nullptr, ParticleMeshType::Square);
	sparks->SetEmitMode(false, 0.05f);
	sparks->SetEmitterParam(50, 0.01f);
	enemyDeathParticle_.push_back(std::move(sparks));

	// 中心フラッシュ
	auto flash = std::make_unique<Particle>();
	flash->Initialize(ctx_.engineContext, pos, "AttractEffect.png", std::make_unique<HitFlashModule>(), nullptr, ParticleMeshType::Square);
	flash->SetEmitMode(false, 0.05f);
	flash->SetEmitterParam(4, 0.01f);
	enemyDeathParticle_.push_back(std::move(flash));

	// 衝撃波リング
	auto ringWave = std::make_unique<Particle>();
	ringWave->Initialize(ctx_.engineContext, pos, "gradationLine.png", std::make_unique<HitRingModule>(), nullptr, ParticleMeshType::Cylinder);
	ringWave->SetEmitMode(false, 0.05f);
	ringWave->SetEmitterParam(1, 0.01f);
	enemyDeathParticle_.push_back(std::move(ringWave));
}

void GamePlayScene::DebugInput() {
#if _DEBUG || NDEBUG
	if (ctx_.keyboard->KeyTriggered(DIK_F1)) {
		ctx_.sceneManager->ChangeScene("EasingEditorScene");
	}

	if (ctx_.keyboard->KeyTriggered(DIK_F2)) {
		isDebugCameraActive_ = !isDebugCameraActive_;

		if (isDebugCameraActive_) {
			ctx_.currentCamera->SetTranslation({0.0f, 200.0f, 0.0f});
		} else {
			ctx_.currentCamera->SetTranslation({player_->GetPosition().x, 60.0f, player_->GetPosition().z});
			ctx_.currentCamera->SetPivot(currentCameraPivot_);
		}
	}

	if (ctx_.keyboard->KeyTriggered(DIK_F3)) {
		enemyManager_->SetMove();
	}

	if (ctx_.keyboard->KeyTriggered(DIK_F4)) {
		enemyManager_->SetStop();
	}

	if (ctx_.keyboard->KeyTriggered(DIK_SPACE) && cameraZoomController_->GetIsActive()) {
		cameraZoomController_->Skip();
	}
#endif
}

void GamePlayScene::UpdatePicking() {
	// ImGuizmo操作中（ギズモをドラッグ中）はクリック判定を行わない
	if (ImGuizmo::IsOver() || ImGuizmo::IsUsing()) {
		return;
	}

	// 左クリックされた瞬間を検知
	if (ctx_.keyboard->MouseButtonTriggered(0)) {

		// マウスのカーソル座標を取得してクライアント座標に変換
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		auto hwnd = ctx_.engineContext->object3dCommon->GetDxCommon()->GetWinApp()->GetHWND();
		ScreenToClient(hwnd, &cursorPos);

		// 画面座標をNDCに変換
		float screenWidth = 1280.0f; // 画面幅
		float screenHeight = 720.0f; // 画面高さ
		float nx = (2.0f * cursorPos.x) / screenWidth - 1.0f;
		float ny = 1.0f - (2.0f * cursorPos.y) / screenHeight;

		// ViewProjectionの逆行列を生成
		Matrix4x4 viewMat = ctx_.currentCamera->GetViewMatrix();
		Matrix4x4 projMat = ctx_.currentCamera->GetProjection();
		Matrix4x4 vpMat = MathUtility::Multiply(viewMat, projMat);
		Matrix4x4 vpInv = MathUtility::Inverse(vpMat);

		// Rayの生成
		Vector3 a = {nx, ny, 0.0f};
		Vector3 b = {nx, ny, 1.0f};
		Vector3 nearPoint = MathUtility::Transform(a, vpInv);
		Vector3 farPoint = MathUtility::Transform(b, vpInv);

		Ray ray;
		ray.origin = nearPoint;
		ray.direction = MathUtility::Normalize(farPoint - nearPoint);

		// オブジェクトとの当たり判定
		float minDistance = FLT_MAX;
		IGameObject* hitObject = nullptr;

		for (IGameObject* obj : objects_) {
			AABB aabb = obj->GetAABBForGizmo();
			float distance = 0.0f;

			// レイとAABBが交差し、かつ今までのオブジェクトより手前にあるか
			if (Collision::Intersect(ray, aabb, distance)) {
				if (distance < minDistance) {
					minDistance = distance;
					hitObject = obj;
				}
			}
		}

		// クリックしたオブジェクトを選択状態にする
		if (hitObject) {
			selectedGameObject_ = hitObject;
		} else {
			// 何もないところをクリックしたら選択解除
			selectedGameObject_ = nullptr;
		}
	}
}