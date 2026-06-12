#include "GamePlayScene.h"
#include "Collision.h"
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

	// カメラの設定
	ctx_.currentCamera = mainCamera_.get();
	ctx_.engineContext->object3dCommon->SetDefaultCamera(ctx_.currentCamera);
	ctx_.engineContext->particleCommon->SetDefaultCamera(ctx_.currentCamera);

	// プレイヤーの初期化
	player_->Initialize(ctx_.engineContext);

	// 敵の生成&初期化
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize(ctx_.engineContext);

	// 敵の弾管理インスタンス生成
	enemyBulletManager_ = std::make_unique<EnemyBulletManager>();

	// 壁の管理インスタンス生成&初期化
	wallManager_ = std::make_unique<WallManager>();
	wallManager_->Initialize(ctx_.engineContext);

	// ゴール判定インスタンス生成&初期化
	goal_ = std::make_unique<Goal>();
	goal_->Initialize(ctx_.engineContext);

	// プレイヤーのHPゲージ生成&初期化
	playerHPGauge_ = std::make_unique<PlayerHPGauge>();
	playerHPGauge_->Initialize(ctx_.engineContext);
	playerHPGauge_->HPBarSpriteApply(static_cast<int>(player_->GetCurrentHP()), static_cast<int>(player_->GetMaxHP()));

	// objects_.push_back(player_->GetObject3d());

	// イージングエディターの生成
	easingEditor_ = std::make_unique<EasingEditor>();

	// フラッシュエフェクトの生成&初期化
	flashEffect_ = std::make_unique<FlashEffect>();
	flashEffect_->Initialize(ctx_.engineContext);

	// レターボックスの生成&初期化
	letterBox_ = std::make_unique<LetterBox>();
	letterBox_->Initialize(ctx_.engineContext);

	// 地面の生成&初期化
	ground_ = std::make_unique<Ground>();
	ground_->Initialize(ctx_.engineContext);

	// シーン遷移要求制御変数
	isTransitionRequested_ = false;

	// スカイボックスの生成&初期化
	skybox_ = std::make_unique<Skybox>();
	skybox_->Initialize(ctx_.engineContext, "rostock_laage_airport_4k.dds");

	controls_ = std::make_unique<Controls>();
	controls_->Initialize(ctx_.engineContext);

	// VignetteのColorを赤に指定
	ctx_.engineContext->copyImage->SetVignetteColor({1, 0, 0, 1});
}

void GamePlayScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();
	
	// 経過時間の記録
	elapsedTime_ += deltaTime;
	ctx_.engineContext->copyImage->SetTime(elapsedTime_);

	// ポーズ画面
	if (ctx_.keyboard->KeyTriggered(DIK_TAB)) {
		RequestScenePush("Pause");
		return;
	}

	// ゴールしていなければクリアタイムを加算
	if (!goal_->GetGoal() && !player_->IsDead()) {
		commonData_->clearTime += deltaTime;
	}

	// Projectionの逆行列をCopyImageに渡す
	Matrix4x4 projInv = MathUtility::Inverse(ctx_.currentCamera->GetProjection());
	ctx_.engineContext->copyImage->SetProjectionInverse(projInv);

	// HP量に応じてVignetteのパラメータを変更
	float hpRatio = player_->GetCurrentHP() / player_->GetMaxHP();

	if (hpRatio < 0.3f) {
		float intensity = (0.3f - hpRatio) / 0.3f;
		ctx_.engineContext->copyImage->SetVignetteIntensity(intensity);
	} else {
		ctx_.engineContext->copyImage->SetVignetteIntensity(0.0f);
	}

	// プレイヤーの更新処理
	player_->Update(deltaTime, ctx_.keyboard, ctx_.gamePad, enemyManager_.get());

	// 敵の更新処理
	enemyManager_->Update(deltaTime, player_.get(), enemyBulletManager_.get(), wallManager_.get());

	// 敵の弾の更新処理
	enemyBulletManager_->Update(deltaTime);

	// 壁の管理インスタンス更新
	wallManager_->Update();

	// ゴール判定インスタンス更新
	goal_->Update();

	// スカイボックス更新
	skybox_->Update(mainCamera_->GetViewMatrix(), mainCamera_->GetProjection());

	// 当たり判定
	CollisionGameObjects();

	// 押し戻し完了後の最終的な座標で、描画更新&AABB更新
	player_->PostUpdate();
	enemyManager_->PostUpdate();

	// ゴールしていたらフラッシュ開始
	if (goal_->GetGoal()) {
		flashEffect_->Trigger();
	}

	// フラッシュの演出が終わったらレターボックスの出現
	if (flashEffect_->Finish()) {
		letterBox_->Trigger();
	}

	// レターボックスの演出が終わったらシーン遷移
	if (!letterBox_->GetIsActive()) {
		if (!isTransitionRequested_) {
			RequestSceneChange("Result");
			isTransitionRequested_ = true;
		}
	}

	// プレイヤーが死亡したらシーン遷移
	if (player_->IsDead()) {
		if (!isTransitionRequested_) {
			RequestSceneChange("Result");
			isTransitionRequested_ = true;
		}
	}

	// 壁の管理インスタンスImGui
	wallManager_->DrawImGui();

	// 敵の管理インスタンスImGui
	enemyManager_->DrawImGui();

	// プレイヤーのHPゲージImGui
	playerHPGauge_->DrawImGui();

	// ギズモ用ImGui更新
	UpdateImGui();

	// デバッグ用のImGui更新
	UpdateDebugImGui();

	// プレイヤーのHPゲージ更新
	playerHPGauge_->HPBarSpriteApply(static_cast<int>(player_->GetCurrentHP()), static_cast<int>(player_->GetMaxHP()));
	playerHPGauge_->Update(deltaTime);

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
		ctx_.currentCamera->SetPivot(player_->GetPosition());
	}

	// カメラのシェイク更新
	ctx_.currentCamera->ShakeCamera(deltaTime);

	// カメラの更新
	ctx_.currentCamera->Update(*ctx_.keyboard, *ctx_.gamePad);

	// イージングエディター更新処理
	easingEditor_->DrawWindow(deltaTime);

	// フラッシュエフェクト更新
	flashEffect_->Update(deltaTime);

	// レターボックス更新
	letterBox_->Update(deltaTime);

	// 地面の更新
	ground_->Update();

	controls_->Update();

#ifdef USE_IMGUI
	Vector3 rot = ctx_.currentCamera->GetEuler();

	ImGui::Begin("Camera");
	ImGui::DragFloat("Pitch", &rot.x, 0.01f);
	ImGui::DragFloat("Yaw", &rot.y, 0.01f);
	ImGui::DragFloat3("Translate", &ctx_.currentCamera->GetTranslation().x, 0.01f);
	ImGui::End();

	ctx_.currentCamera->SetEuler(rot);

	Vector3 position = skybox_->GetPosition();
	Vector3 scale = skybox_->GetScale();

	ImGui::Begin("Skybox");
	ImGui::DragFloat3("Position", &position.x, 0.01f);
	ImGui::DragFloat3("Scale", &scale.x, 0.01f);
	ImGui::End();

	skybox_->SetPosition(position);
	skybox_->SetScale(scale);
#endif // USE_IMGUI

#ifdef _DEBUG
	if (ctx_.keyboard->KeyTriggered(DIK_F1)) {
		ctx_.sceneManager->ChangeScene("EasingEditorScene");
	}

	if (ctx_.keyboard->KeyTriggered(DIK_F2)) {
		isDebugCameraActive_ = !isDebugCameraActive_;

		if (isDebugCameraActive_) {
			ctx_.currentCamera->SetTranslation({0.0f, 250.0f, 0.0f});
		} else {
			ctx_.currentCamera->SetTranslation({0.0f, 60.0f, 0.0f});
		}
	}
#endif // _DEBUG
}

void GamePlayScene::Draw() {
	// プレイヤーの描画処理
	player_->Draw();

	// 敵の描画処理
	enemyManager_->Draw();

	// 敵の弾の描画処理
	enemyBulletManager_->Draw();

	// 壁の管理インスタンス描画
	wallManager_->Draw();

	// ゴール判定インスタンス描画
	goal_->Draw();

	// プレイヤーのHPゲージ描画
	playerHPGauge_->Draw();

	// 地面の描画
	ground_->Draw();

	// パーティクルの描画
	for (auto& particle : enemyDeathParticle_) {
		particle->Draw();
	}

	// フラッシュエフェクト描画
	flashEffect_->Draw();

	// レターボックス描画
	letterBox_->Draw();

	controls_->Draw();
}

void GamePlayScene::Finalize() {}

void GamePlayScene::CollisionGameObjects() {
	// ==========================================
	// プレイヤーと敵の弾の当たり判定
	// ==========================================
	AABB playerCol = player_->GetBodyCol();
	for (const auto& bullet : enemyBulletManager_->GetBullets()) {
		// 衝突判定
		if (Collision::Intersect(playerCol, bullet->GetCollision())) {
			// プレイヤーにダメージを与える処理
			player_->Damage(1.0f);

			// カメラシェイク開始
			ctx_.currentCamera->StartShake(0.2f, 0.2f);
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
					auto particle = std::make_unique<Particle>();
					particle->Initialize(ctx_.engineContext, enemy->GetPos(), "gradationLine.png", std::make_unique<ShockWaveModule>(), nullptr, TinyEngine::ParticleMeshType::Cylinder);
					particle->SetEmitMode(false, 0.1f);
					particle->SetEmitterParam(20, 0.05f);
					enemyDeathParticle_.push_back(std::move(particle));

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
					a->Kill();
					b->Kill();
					commonData_->killCount += 2;

					// パーティクルの生成
					auto particle = std::make_unique<Particle>();
					particle->Initialize(ctx_.engineContext, a->GetPos(), "gradationLine.png", std::make_unique<ShockWaveModule>(), nullptr, TinyEngine::ParticleMeshType::Cylinder);
					particle->SetEmitMode(false, 0.1f);
					particle->SetEmitterParam(20, 0.05f);
					enemyDeathParticle_.push_back(std::move(particle));
				}
			}
		}
	}
}

void GamePlayScene::UpdateImGui() {
#ifdef USE_IMGUI
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

	// リストから選択
	if (ImGui::BeginListBox("Objects")) {
		for (auto& obj : objects_) {
			std::string label = obj->GetName() + " (ID: " + std::to_string(obj->GetID()) + ")";
			bool isSelected = (selectedObject_ == obj);
			if (ImGui::Selectable(label.c_str(), isSelected)) {
				selectedObject_ = obj;
			}
		}
		ImGui::EndListBox();
	}
	ImGui::End();

	// ギズモ描画レイヤー
	if (selectedObject_ != nullptr) {
		// ウィンドウ位置とサイズを画面全体に固定
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always); // 解像度に合わせて調整

		// 背景を完全に透明にする設定
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::Begin("GizmoLayer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar);

		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::Enable(true);
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList()); // 現在のウィンドウのDrawListを使用

		// 描画範囲の設定
		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

		// 選択中オブジェクトのギズモを描画
		selectedObject_->DrawGizmo(ctx_.currentCamera->GetViewMatrix(), ctx_.currentCamera->GetProjection(), currentGizmoOperation_, ImGuizmo::LOCAL);

		ImGui::End();
		ImGui::PopStyleColor();
	}
#endif // USE_IMGUI
}

void GamePlayScene::UpdateDebugImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Debug List");
	if (ImGui::Button("Kill Player")) {
		player_->Damage(1000.0f);
	}
	ImGui::End();
#endif
}
