#include "GamePlayScene.h"
#include "GameObjects/Effect/EffectGenerator.h"
#include "SceneManager.h"

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

	// ライトの設定
	DirectionalLight dirLight;
	dirLight.color = {1,1,1,1};
	dirLight.intensity = 2.0f;
	dirLight.direction = {0.5f, 0.5f, -0.5f};
	ctx.engineContext->object3dCommon->SetDirectionalLightParam(dirLight);

	// メインカメラ
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize();
	mainCamera_->SetTranslation({0.0f, 60.0f, 0.0f});
	mainCamera_->SetPivot(player_->GetPosition());
	mainCamera_->SetEuler({std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f});
	currentCameraPivot_ = player_->GetPosition();
	cameraPosY_ = -20.0f;

	// カメラの設定
	ctx_.currentCamera = mainCamera_.get();
	ctx_.engineContext->object3dCommon->SetDefaultCamera(ctx_.currentCamera);
	ctx_.engineContext->particleCommon->SetDefaultCamera(ctx_.currentCamera);

	// デカール管理インスタンス
	decalManager_ = std::make_unique<DecalManager>();
	decalManager_->Initialize(ctx.engineContext);

	// プレイヤーの初期化
	player_->Initialize(ctx_.engineContext, decalManager_.get());

	// 敵の生成&初期化
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize(ctx_.engineContext, decalManager_.get());

	// 敵の弾管理インスタンス生成
	enemyBulletManager_ = std::make_unique<EnemyBulletManager>();

	// 敵の爆弾管理インスタンス生成
	enemyBombManager_ = std::make_unique<EnemyBombManager>();

	// フラッシュエフェクトの生成&初期化
	flashEffect_ = std::make_unique<FlashEffect>();
	flashEffect_->Initialize(ctx_.engineContext);

	// レターボックスの生成&初期化
	letterBox_ = std::make_unique<LetterBox>();
	letterBox_->Initialize(ctx_.engineContext);

	// ステージ管理インスタンスの生成&初期化
	stage_ = std::make_unique<Stage>();
	stage_->Initialize(ctx, decalManager_.get());

	// シーン遷移要求制御変数
	isTransitionRequested_ = false;

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

	// シーンエディターの生成&初期化
	sceneEditor_ = std::make_unique<SceneEditor>();
	sceneEditor_->Initialize();

	// 当たり判定管理インスタンス生成
	collisionManager_ = std::make_unique<CollisionManager>();

	// 操作方法UI
	controlUI_ = std::make_unique<ControlUI>();
	controlUI_->Initialize(ctx.engineContext, decalManager_.get());
	controlUI_->AddAttackUIDecal({2, 2, 2});
	controlUI_->AddHoldUIDecal({2, 2, 2});
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
	if (!stage_->GetGoal()->GetIsGoal() && !player_->IsDead()) {
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
	enemyManager_->Update(deltaTime, player_.get(), enemyBulletManager_.get(), stage_->GetWallManager(), stage_->GetDoorManager(), stage_->GetGlassManager(), enemyBombManager_.get());

	// 敵の弾の更新処理
	enemyBulletManager_->Update(deltaTime);

	// 敵の爆弾の更新処理
	enemyBombManager_->Update(deltaTime);

	// ステージオブジェクトの更新
	stage_->Update(deltaTime, player_->GetPosition(), ctx_.currentCamera);

	// 当たり判定
	collisionManager_->CheckCollisions(
	    player_.get(), enemyManager_.get(), enemyBulletManager_.get(), enemyBombManager_.get(), stage_.get(), ctx_.currentCamera, commonData_, [this](const Vector3& pos) { GenerateEnemyDeathEffect(pos); }, glitchTimer_);

	// 押し戻し完了後の最終的な座標で、描画更新&AABB更新
	player_->PostUpdate();
	enemyManager_->PostUpdate();

	// ゴールしていたらフラッシュ開始
	if (stage_->GetGoal()->GetIsGoal()) {
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

	// ステージオブジェクト管理インスタンスImGui
	stage_->DrawImGui();

	// 敵の管理インスタンスImGui
	enemyManager_->DrawImGui();

	// 操作方法UIの更新&ImGui
	controlUI_->Update();
	controlUI_->DrawImGui();

	// プレイヤーのImGui
	player_->DrawImGui();

	// 毎フレームオブジェクトリストを生成してシーンエディターに渡す
	std::vector<IGameObject*> editObjects;
	editObjects.push_back(player_.get());
	editObjects.push_back(stage_->GetGoal());
	for (auto& enemy : enemyManager_->GetEnemies()) {
		editObjects.push_back(enemy.get());
	}
	for (auto& wall : stage_->GetWallManager()->GetWalls()) {
		editObjects.push_back(wall.get());
	}
	for (auto& door : stage_->GetDoorManager()->GetDoors()) {
		editObjects.push_back(door.get());
	}
	for (auto& glass : stage_->GetGlassManager()->GetGlasses()) {
		editObjects.push_back(glass.get());
	}
	for (auto& cage : stage_->GetCageManager()->GetCages()) {
		editObjects.push_back(cage.get());
	}
	for (auto& healArea : stage_->GetHealAreaManager()->GetHealAreas()) {
		editObjects.push_back(healArea.get());
	}

	// シーンエディターの更新
	sceneEditor_->Update(ctx_, editObjects, player_.get(), enemyManager_.get(), cameraZoomController_.get(), cameraPosY_, isDebugCameraActive_, currentCameraPivot_);

	// パーティクルの更新
	for (auto& particle : enemyDeathEffect_) {
		particle->Update();
	}

	std::erase_if(enemyDeathEffect_, [this](const std::unique_ptr<TinyEngine::Particle>& p) {
		bool isFinished = p->IsFinished();
		if (isFinished) {
			// パーティクルが終了して削除される瞬間だけカメラを揺らす
			ctx_.currentCamera->StartShake(0.2f, 0.3f);
		}
		return isFinished;
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
		float followSpeed = 5.0f;
		currentCameraPivot_.x += (targetPivot.x - currentCameraPivot_.x) * followSpeed * deltaTime;
		currentCameraPivot_.y = cameraPosY_;
		currentCameraPivot_.z += (targetPivot.z - currentCameraPivot_.z) * followSpeed * deltaTime;

		ctx_.currentCamera->SetPivot(currentCameraPivot_);

		// 進行方向にカメラを傾ける処理
		float maxTiltAngle = cameraAngle_ * (std::numbers::pi_v<float> / 180.0f);

		// 基準のピッチ角
		float basePitch = std::numbers::pi_v<float> / 2.0f;

		// プレイヤーの向き(ヨー角)から、カメラのピッチ(X回転)とロール(Z回転)の目標値を計算
		float targetPitch = basePitch - std::cos(playerRot.y) * maxTiltAngle;
		float targetRoll = std::sin(playerRot.y) * maxTiltAngle;

		// 現在のカメラの角度を取得
		Vector3 currentEuler = ctx_.currentCamera->GetEuler();

		currentEuler.x += (targetPitch - currentEuler.x) * tiltSpeed_ * deltaTime;
		currentEuler.y = 0.0f;
		currentEuler.z += (targetRoll - currentEuler.z) * tiltSpeed_ * deltaTime;

		// カメラに角度を適用
		ctx_.currentCamera->SetRotate(currentEuler);
	}

	// プレイヤーが敵を掴んだらシェイク
	if (player_->GetIsGrabTriggerd()) {
		ctx_.currentCamera->StartShake(0.2f, 0.2f);
		player_->SetIsGrabTriggerd(false);
	}

	// カメラのシェイク更新
	ctx_.currentCamera->ShakeCamera(deltaTime, shakePower_);

	// カメラの更新
	ctx_.currentCamera->Update(*ctx_.keyboard, *ctx_.gamePad);

	// フラッシュエフェクト更新
	flashEffect_->Update(deltaTime);

	// レターボックス更新
	letterBox_->Update(deltaTime);

	// 血痕管理インスタンス更新
	decalManager_->SetCamera(ctx_.currentCamera);
	decalManager_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("Camera");
	ImGui::DragFloat("angle", &cameraAngle_, 0.01f);
	ImGui::DragFloat("speed", &tiltSpeed_, 0.01f);
	ImGui::End();
#endif
}

void GamePlayScene::Draw() {
	// 不透明オブジェクトの描画準備
	ctx_.engineContext->object3dCommon->DrawSettingCommon(ctx_.engineContext->textureManager);

	// ステージオブジェクトの描画
	stage_->Draw();

	// 敵の描画処理
	enemyManager_->Draw();

	// 敵の弾の描画処理
	enemyBulletManager_->Draw();

	// 敵の爆弾の描画処理
	enemyBombManager_->Draw();

	// プレイヤーの描画処理
	player_->Draw();

	// パーティクルの描画
	for (auto& particle : enemyDeathEffect_) {
		particle->Draw();
	}

	// 血痕描画
	decalManager_->Draw();

	// フラッシュエフェクト描画
	flashEffect_->Draw();

	// レターボックス描画
	letterBox_->Draw();

	// 半透明オブジェクトの描画準備
	ctx_.engineContext->object3dCommon->DrawSettingTransparent(ctx_.engineContext->textureManager);

	// ガラスの描画
	stage_->DrawTransparent();
}

void GamePlayScene::Finalize() {
	// シーン終了時にエフェクトをデフォルトに戻す
	ctx_.engineContext->postEffectPipeline->SetEffects({PostEffectType::FullScreen});
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

void GamePlayScene::GenerateEnemyDeathEffect(const Vector3& pos) {
	// エフェクトの生成
	EffectGenerator::CreateEnemyDeathEffect(ctx_.engineContext, pos, enemyDeathEffect_);
}