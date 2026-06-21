#include "Player.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "HitFlashModule.h"
#include "HitRingModule.h"
#include "HitSparkModule.h"

using namespace TinyEngine;

void Player::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, std::numbers::pi_v<float> / 2.0f, 0.0f};
	transform_.translate = {0.0f, 0.0f, 0.0f};

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "suzanne.obj");
	render_->SetColor(color_);
	render_->SetTransform(transform_);
	render_->SetEnvScale(envScale_);

	// 移動用インスタンス生成
	move_ = std::make_unique<PlayerMove>();

	// HP管理用インスタンス生成&初期化
	hp_ = std::make_unique<PlayerHealth>();
	hp_->Initialize(maxHP_);

	// パーティクル生成タイマー初期化
	particleGenerateTimer_.Initialize(0.2f);
}

void Player::Update(float deltaTime, DirectInput* input, GamePad* gamePad, EnemyManager* enemyManager) {
	// アナログ入力ベクトル
	Vector2 inputDir = {0.0f, 0.0f};

	if (input->KeyDown(DIK_D))
		inputDir.x += 1.0f;
	if (input->KeyDown(DIK_A))
		inputDir.x -= 1.0f;
	if (input->KeyDown(DIK_W))
		inputDir.y += 1.0f;
	if (input->KeyDown(DIK_S))
		inputDir.y -= 1.0f;

	// ゲームパッド入力
	if (gamePad && gamePad->GetState().connected) {
		const auto& padState = gamePad->GetState();

		// 十字キー
		if (padState.buttons.dpadRight)
			inputDir.x += 1.0f;
		if (padState.buttons.dpadLeft)
			inputDir.x -= 1.0f;
		if (padState.buttons.dpadUp)
			inputDir.y += 1.0f;
		if (padState.buttons.dpadDown)
			inputDir.y -= 1.0f;

		// 左スティック入力（デッドゾーンを超えていれば上書き）
		if (std::abs(padState.axes.lx) > 0.1f || std::abs(padState.axes.ly) > 0.1f) {
			inputDir.x = padState.axes.lx;
			inputDir.y = -padState.axes.ly;
		}
	}

	// ベクトルの長さを計算し、斜め移動時などに長さが1.0を超えないようにクランプ
	float length = std::sqrtf(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
	if (length > 1.0f) {
		inputDir.x /= length;
		inputDir.y /= length;
	}

	// 移動状態の更新と向きの記録
	if (length > 0.05f) {
		// 正規化して記録（攻撃・投げの方向用）
		lastMoveDirection_ = {inputDir.x / length, inputDir.y / length};
		isMoving_ = true;
	} else {
		isMoving_ = false;
	}

	// 敵を掴んでいる場合は速度を半減させる
	speedMultiplier_ = isHold_ ? 0.5f : 1.0f;

	if (!hp_->IsDead()) {
		// 移動更新
		move_->Update(&transform_, inputDir, deltaTime, speedMultiplier_);
	}

	// HP管理インスタンス更新
	hp_->Update(deltaTime);

	// HP管理用インスタンスImGui
	hp_->DrawImGui();

	// 衝突判定のために、移動後の座標で仮のAABBの更新
	UpdateCollision();

	// 掴み判定用：一番近い敵を探す
	Enemy* targetEnemy = nullptr;
	float minDist = FLT_MAX;

	for (auto& enemy : enemyManager->GetEnemies()) {
		if (enemy->IsDead())
			continue;

		// プレイヤーと敵の距離を計算
		Vector3 ePos = enemy->GetPos();
		Vector3 diff = {ePos.x - transform_.translate.x, ePos.y - transform_.translate.y, ePos.z - transform_.translate.z};
		float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

		if (dist < minDist && dist < grabRange_) {
			minDist = dist;
			heldEnemy_ = enemy.get();
		}
	}

	bool isGrabTriggered = input->KeyDown(DIK_J);
	bool isGrabReleased = input->KeyReleased(DIK_J);
	bool isAttackTriggered = input->KeyTriggered(DIK_K);

	if (gamePad && gamePad->GetState().connected) {
		const auto& padState = gamePad->GetState();

		// Lトリガーで掴み
		if (padState.axes.lt > 0.3f) {
			isGrabTriggered = true;
		}
		// 放す
		if (padState.buttonsReleased.x || padState.buttonsReleased.a) {
			isGrabReleased = true;
		}
		// Rトリガーで突き飛ばし
		if (padState.axes.rt > 0.3f) {
			isAttackTriggered = true;
		}
	}

	// 掴み・投げ処理の更新
	if (enableAttack_ && isGrabTriggered) {
		if (!isHold_ && heldEnemy_ != nullptr) {
			isHold_ = true;
			heldEnemy_->SetEnableMove(false);
		}

		// 掴んでいる間はプレイヤーの位置に敵を固定
		if (isHold_ && heldEnemy_ != nullptr) {
			Vector3 forward = {lastMoveDirection_.x, 0.0f, lastMoveDirection_.y};

			// 前方オフセット
			float offset = 0.8f;
			Vector3 holdPos = {transform_.translate.x + forward.x * offset, transform_.translate.y, transform_.translate.z + forward.z * offset};

			heldEnemy_->SetPos(holdPos);
			heldEnemy_->SetRotate(transform_.rotate);
		}
	} else if (isHold_ && isGrabReleased) { // 手放す処理
		isHold_ = false;
		if (heldEnemy_) {
			heldEnemy_->SetEnableMove(true);
			heldEnemy_ = nullptr; // 手放す
		}
	}

	// 投げる・攻撃処理
	if (enableAttack_ && isAttackTriggered && heldEnemy_ != nullptr) {
		isHold_ = false;
		enableAttack_ = false;
		heldEnemy_->SetEnableMove(true);
		heldEnemy_->StartKnockBack({lastMoveDirection_.x, 0.0f, lastMoveDirection_.y});
		heldEnemy_ = nullptr; // 投げたのでポインタをクリア
	}

	// パーティクルの生成タイマー更新
	if (isMoving_) {
		particleGenerateTimer_.Update(deltaTime);
	}

	// パーティクルの生成
	if (particleGenerateTimer_.IsEnd() && !hp_->IsDead()) {
		auto particle = std::make_unique<Particle>();
		particle->Initialize(ctx_, transform_.translate, "Dust.png", std::make_unique<DustModule>(), nullptr, TinyEngine::ParticleMeshType::Square);
		particle->SetEmitMode(false, 0.1f);
		particle->SetEmitterParam(5, 0.1f);
		dustParticle_.push_back(std::move(particle));

		// タイマーの再設定
		particleGenerateTimer_.Initialize(0.2f);
	}

	// パーティクルの更新
	for (auto& particle : dustParticle_) {
		particle->Update();
	}

	// パーティクル削除処理
	std::erase_if(dustParticle_, [this](const std::unique_ptr<TinyEngine::Particle>& p) { return p->IsFinished(); });

	// ヒットエフェクトの更新
	for (auto& particle : hitEffects_) {
		particle->Update();
	}

	// ヒットエフェクト削除
	std::erase_if(hitEffects_, [](const std::unique_ptr<TinyEngine::Particle>& p) { return p->IsFinished(); });

#ifdef USE_IMGUI
	ImGui::Begin("Player");
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
	ImGui::SliderFloat("EnvScale", &envScale_, 0.0f, 1.0f);
	ImGui::ColorEdit4("Color", &color_.x);
	ImGui::End();
	render_->SetEnvScale(envScale_);
	render_->SetColor(color_);
#endif // USE_IMGUI
}

void Player::PostUpdate() {
	// 押し戻しによってtransform_が変更された場合、それに合わせてAABBも最終的な正しい位置に合わせる
	UpdateCollision();

	// 最後に描画用の更新を行う
	render_->Update(transform_);
}

void Player::Draw() {
	// パーティクルの描画
	for (auto& particle : dustParticle_) {
		particle->Draw();
	}

	// ヒットエフェクトパーティクルの描画
	for (auto& particle : hitEffects_) {
		particle->Draw();
	}

	// 描画
	render_->Draw();
}

bool Player::IsDead() const { return hp_->IsDead(); }

void Player::Damage(float value) {
	// ダメージ処理
	hp_->Damage(value);

	// ヒットエフェクト生成
	GenerateHitEffect();
}

void Player::UpdateCollision() {
	// 攻撃用の当たり判定更新
	Vector3 pos = transform_.translate;
	attackCol_.max = {pos.x + 0.5f, pos.y, pos.z + 0.5f};
	attackCol_.min = {pos.x - 0.5f, pos.y, pos.z - 0.5f};

	// 本体の当たり判定更新
	bodyCol_.max = {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f};
	bodyCol_.min = {pos.x - 0.5f, pos.y - 0.5f, pos.z - 0.5f};
}

void Player::GenerateHitEffect() {
	if (preHP_ == hp_->GetCurrentHP())
		return;

	Vector3 effectPos = transform_.translate;

	// 飛び散る火花
	auto sparks = std::make_unique<TinyEngine::Particle>();
	sparks->Initialize(ctx_, effectPos, "white.png", std::make_unique<HitSparkModule>(), nullptr, TinyEngine::ParticleMeshType::Square);
	sparks->SetEmitMode(false, 0.05f);
	sparks->SetEmitterParam(50, 0.01f);
	hitEffects_.push_back(std::move(sparks));

	// 中心の閃光フラッシュ
	auto flash = std::make_unique<TinyEngine::Particle>();
	flash->Initialize(ctx_, effectPos, "AttractEffect.png", std::make_unique<HitFlashModule>(), nullptr, TinyEngine::ParticleMeshType::Square);
	flash->SetEmitMode(false, 0.05f);
	flash->SetEmitterParam(4, 0.01f);
	hitEffects_.push_back(std::move(flash));

	// 拡散する衝撃波リング
	auto ringWave = std::make_unique<TinyEngine::Particle>();
	ringWave->Initialize(ctx_, effectPos, "gradationLine.png", std::make_unique<HitRingModule>(), nullptr, TinyEngine::ParticleMeshType::Cylinder);
	ringWave->SetEmitMode(false, 0.05f);
	ringWave->SetEmitterParam(1, 0.01f);
	hitEffects_.push_back(std::move(ringWave));

	preHP_ = hp_->GetCurrentHP();
}