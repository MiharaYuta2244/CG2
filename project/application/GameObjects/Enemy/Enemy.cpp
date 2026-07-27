#include "Enemy.h"
#include "ChargeModule.h"
#include "EnemyBulletManager.h"
#include "GameObjects/Effect/EffectGenerator.h"
#include "GameObjects/Player/Player.h"
#include "ColorPalette.h"

using namespace TinyEngine;

int Enemy::index = 0;

Enemy::Enemy() { id_ = index++; }

void Enemy::Initialize(EngineContext* ctx, Vector3 pos, EnemyType type, DecalManager* bloodDecalManager) {
	ctx_ = ctx;
	type_ = type;

	switch (type_) {
	case EnemyType::Normal:
		hp_ = 1;
		color_ = {1, 1, 1, 1};
		break;

	case EnemyType::Shotgun:
		hp_ = 2;
		color_ = {1, 0, 0, 1};
		break;

	case EnemyType::Bomber:
		hp_ = 1;
		color_ = {0, 0, 1, 1};
		break;
	}

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = pos;

	// 描画用インスタンスの生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Hiyoko.obj");
	render_->SetTransform(transform_);
	render_->SetEnvScale(envScale_);
	render_->SetColor(color_);

	// AIインスタンス生成&初期化
	ai_ = std::make_unique<EnemyAI>();
	ai_->Initialize(&transform_, ctx, type);

	// 視界インスタンス生成&初期化
	visionCone_ = std::make_unique<VisionCone>();
	Visionparam param = ai_->GetVisionParam();
	visionCone_->Initialize(ctx, param.radius, param.angle);

	// 血痕
	bloodDecalManager_ = bloodDecalManager;
}

void Enemy::Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager, DoorManager* doorManager, GlassManager* glassManager, EnemyBombManager* enemyBombManager) {
	if (isDead_) {
		return;
	}

	// ノックバックアニメーション
	if (knockBackAnim_.anim.GetIsActive()) {
		knockBackAnim_.anim.Update(deltaTime, knockBackAnim_.temp);
		transform_.translate = knockBackAnim_.temp;
	}

	if (!isMove_) {
		return;
	}

	Vector3 playerPos = player->GetPosition();
	Vector3 enemyPos = transform_.translate;
	float dx = playerPos.x - enemyPos.x;
	float dy = playerPos.y - enemyPos.y;
	float dz = playerPos.z - enemyPos.z;
	float distSq = dx * dx + dy * dy + dz * dz;

	// 画面に収まる程度の距離
	const float kActiveDistance = 30.0f;
	bool isWithinActiveRange = (distSq <= (kActiveDistance * kActiveDistance));

	if (isWithinActiveRange) {
		// 無敵タイマー更新
		if (invincibleTimer_ > 0.0f) {
			invincibleTimer_ -= deltaTime;
		}

		// AIインスタンス更新
		ai_->Update(deltaTime, player, enemyBulletManager, wallManager, doorManager, glassManager, enemyBombManager);

		if (ai_->IsShotThisFrame()) {
			GenerateMuzzleFlash(ai_->GetShotDirection());
		}

		// プレイヤー発見時に「!」マークの生成
		if (lastState != ai_->GetState() && ai_->GetState() == EnemyAI::State::Vigilance) {
			GenerateExMark();
		}

		// AIの状態を取得して色を変える
		if (ai_->GetState() == EnemyAI::State::Vigilance) {
			// 警戒状態なら赤色
			visionCone_->SetColor({1.0f, 0.0f, 0.0f, 0.3f});
			// 射撃ゲージの進行度を渡す
			visionCone_->SetChargeProgress(ai_->GetShotProgress());
		} else if (ai_->GetState() == EnemyAI::State::Hold) {
			// 拘束状態も射撃するのでプログレスを渡す
			visionCone_->SetColor({1.0f, 0.5f, 0.0f, 0.3f});
			visionCone_->SetChargeProgress(ai_->GetShotProgress());
		} else {
			// 通常状態なら緑色
			visionCone_->SetColor({0.0f, 1.0f, 0.0f, 0.3f});
			// プログレスはリセット
			visionCone_->SetChargeProgress(0.0f);
		}
	}

	// 「!」マークのアニメーションが終了していれば「!」マークインスタンスを削除
	if (exclamationMark_) {
		if (!exclamationMark_->IsFinishedAnimation()) {
			exclamationMark_.reset();
			exclamationMark_ = nullptr;
		}
	}

	// 「!」マークの更新
	if (exclamationMark_) {
		exclamationMark_->Update(deltaTime, transform_.translate);
	}

	// 視界
	visionCone_->SetTranslate(transform_.translate);
	visionCone_->SetRotate(transform_.rotate);
	visionCone_->Update(wallManager->GetObjects(), doorManager->GetObjects(), glassManager->GetObjects());

	// 敵AIの状態を記録
	lastState = ai_->GetState();

	// 当たり判定更新　衝突判定用
	UpdateCollision();

	// 点滅用タイマーを回す
	if (damageBlinkTimer_ > 0.0f) {
		damageBlinkTimer_ -= deltaTime;

		// 0.05秒ごとにON/OFF切り替え
		const float blinkInterval = 0.05f;
		if (fmod(damageBlinkTimer_, blinkInterval * 2) < blinkInterval) {
			isBlinkVisible_ = false;
		} else {
			isBlinkVisible_ = true;
		}
	} else {
		isBlinkVisible_ = true;
	}

	// マズルフラッシュパーティクルの更新と自動削除
	for (auto& particle : muzzleParticles_) {
		particle->Update();
	}
	std::erase_if(muzzleParticles_, [](const std::unique_ptr<Particle>& p) { return p->IsFinished(); });

	// チャージパーティクルの生成
	if (!chargeParticle_ && ai_->GetShotTimer() >= 0.01f) {
		chargeParticle_ = std::make_unique<Particle>();
		chargeParticle_->Initialize(ctx_, transform_.translate, "AttractEffect.png", std::make_unique<ChargeModule>(), nullptr, ParticleMeshType::Square);
		chargeParticle_->SetEmitMode(true, 0.1f);
		chargeParticle_->SetEmitterParam(3, 0.05f);

		// シリンダーパーティクルの生成&初期化
		if (!chargeCylinderParticle_) {
			chargeCylinderParticle_ = std::make_unique<Particle>();
			chargeCylinderParticle_->Initialize(ctx_, transform_.translate, "gradationLine.png", std::make_unique<ChargeCylinderModule>(ai_.get(), &transform_), nullptr, ParticleMeshType::Cylinder);
			chargeCylinderParticle_->SetEmitMode(false, 0.0f);
			chargeCylinderParticle_->SetEmitterParam(1, 9999.0f);
		}
	}

	// チャージパーティクルの更新
	if (chargeParticle_) {
		chargeParticle_->SetTranslate(transform_.translate);
		chargeParticle_->Update();
	}

	if (chargeCylinderParticle_ && (ai_->GetState() == EnemyAI::State::Vigilance || ai_->GetState() == EnemyAI::State::Hold)) {
		chargeCylinderParticle_->Update();
	}

	// チャージパーティクルの削除
	if (chargeParticle_ && (ai_->GetIsShot() || ai_->GetState() == EnemyAI::State::Normal)) {
		chargeParticle_.reset();
		chargeParticle_ = nullptr;

		if (chargeCylinderParticle_) {
			chargeCylinderParticle_.reset();
			chargeCylinderParticle_ = nullptr;
		}
	}

	// ギズモ用当たり判定更新
	UpdateAABBForGizmo();
}

void Enemy::PostUpdate() {
	// 押し戻しを反映したAABBの再計算
	UpdateCollision();

	// 正しい座標で描画用インスタンスの更新
	render_->Update(transform_);
}

void Enemy::Draw() {
	if (!isDead_) {
		// 描画
		if (isBlinkVisible_) {
			render_->Draw();
		}

		// 視界
		if (enableMove_) {
			visionCone_->Draw();
		}

		// 「!」マークの描画
		if (exclamationMark_) {
			exclamationMark_->Draw();
		}

		// マズルフラッシュパーティクルの描画
		for (auto& particle : muzzleParticles_) {
			particle->Draw();
		}

		// チャージパーティクルの描画
		if (chargeParticle_) {
			chargeParticle_->Draw();
		}

		// 描画
		if ((ai_->GetState() == EnemyAI::State::Vigilance || ai_->GetState() == EnemyAI::State::Hold) && chargeCylinderParticle_) {
			chargeCylinderParticle_->Draw();
		}
	}
}

void Enemy::StartKnockBack(Vector3 dir) {
	Vector3 pos = transform_.translate;

	// 入力方向ベクトルを正規化
	if (dir.x != 0.0f || dir.z != 0.0f) {
		dir = MathUtility::Normalize(dir);
	} else {
		return;
	}

	Vector3 targetPos = {pos.x + dir.x * knockBackPower_, pos.y, pos.z + dir.z * knockBackPower_};
	knockBackAnim_.anim.Start(transform_.translate, targetPos, 0.5f, EaseType::EASEOUTCUBIC);
}

void Enemy::Kill() {
	if (isDead_)
		return;

	isDead_ = true;

	// 血痕の生成
	AddBloodDecal();
}

void Enemy::Damage() {
	if (isDead_)
		return;

	// 無敵時間中は早期リターン
	if (invincibleTimer_ > 0.0f) {
		return;
	}

	hp_--;
	if (hp_ <= 0) {
		Kill();
	} else {
		ai_->SetState(EnemyAI::State::Normal);
		ai_->ResetShotTimer();

		// 血痕の生成
		AddBloodDecal();
	}

	damageBlinkTimer_ = 1.0f; // 点滅させる時間を設定
	isBlinkVisible_ = true;   // 点滅用のフラグを立てる
	invincibleTimer_ = 0.5f;  // 無敵時間の設定
}

void Enemy::UpdateCollision() {
	Vector3 pos = transform_.translate;
	bodyCol_.max = {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f};
	bodyCol_.min = {pos.x - 0.5f, pos.y - 0.5f, pos.z - 0.5f};
}

void Enemy::GenerateExMark() {
	// 「!」マークインスタンス生成&初期化
	exclamationMark_ = std::make_unique<ExclamationMark>();
	Vector3 pos = {transform_.translate.x, transform_.translate.y + 1.0f, transform_.translate.z};
	exclamationMark_->Initialize(ctx_, transform_.translate);
}

void Enemy::GenerateMuzzleFlash(const Vector3& direction) {
	Vector3 muzzlePos = transform_.translate + direction * 1.2f;

	// エフェクトの生成
	EffectGenerator::CreateMuzzleFlash(ctx_, muzzlePos, direction, muzzleParticles_);
}

void Enemy::SetAIState(EnemyAI::State state) {
	if (ai_) {
		ai_->SetState(state);
	}
}

void Enemy::StopAnimation() {
	if (knockBackAnim_.anim.GetIsActive()) {
		knockBackAnim_.anim.Reset();
	}
}

void Enemy::AddBloodDecal() {
	Vector3 basePos = transform_.translate;
	Vector3 finalPos;
	finalPos.x = basePos.x;
	finalPos.y = 0.1f;
	finalPos.z = basePos.z;
	bloodDecalManager_->AddDecal("Bleeding.png", finalPos, {std::numbers::pi_v<float> / 2.0f, 0, 0}, {4, 4, 1}, ColorPalette::DarkRed());
}

void Enemy::SetEnemyType(EnemyType type) {
	type_ = type;

	// タイプに応じてステータスを変更
	switch (type_) {
	case EnemyType::Normal:
		hp_ = 1;
		color_ = {1, 1, 1, 1};
		break;

	case EnemyType::Shotgun:
		hp_ = 2;
		color_ = {1, 0, 0, 1};
		break;

	case EnemyType::Bomber:
		hp_ = 1;
		color_ = {0, 0, 1, 1};
		break;
	}

	// 描画用の色を更新
	if (render_) {
		render_->SetColor(color_);
	}

	// AIと視界を再初期化
	if (ai_ && visionCone_) {
		ai_->Initialize(&transform_, ctx_, type_);
		Visionparam param = ai_->GetVisionParam();
		visionCone_->Initialize(ctx_, param.radius, param.angle);
	}
}