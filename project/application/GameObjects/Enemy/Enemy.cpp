#include "Enemy.h"
#include "ChargeModule.h"
#include "EnemyBulletManager.h"
#include "GameObjects/Effect/EffectGenerator.h"
#include "GameObjects/Player/Player.h"
#include "ColorPalette.h"

using namespace TinyEngine;

int Enemy::index = 0;

Enemy::Enemy() { id_ = index++; }

void Enemy::Initialize(EngineContext* ctx, Vector3 pos, EnemyType type, DecalManager* bloodDecalManager, AudioManager* audioManager) {
	ctx_ = ctx;
	type_ = type;
	audioManager_ = audioManager;

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

	transform_.scale = {0.2f, 0.2f, 0.2f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = pos;

	// 描画用インスタンスの生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "HumanIdle.gltf");
	render_->SetTransform(transform_);
	render_->SetEnvScale(envScale_);
	render_->SetColor(color_);
	render_->SetIsSkinning(true);
	KeyframeAnimation keyframeAnimation;
	Animation idleAnimation = keyframeAnimation.LoadAnimationFile("HumanIdle.gltf");
	render_->GetObject3d()->PlayAnimation(idleAnimation);

	renderGun_ = std::make_unique<ObjectRender>();
	renderGun_->Initialize(ctx, "Cube.obj");
	renderGun_->SetTransform(transform_);
	renderGun_->SetEnvScale(envScale_);
	renderGun_->SetColor(color_);

	// AIインスタンス生成&初期化
	ai_ = std::make_unique<EnemyAI>();
	ai_->Initialize(&transform_, ctx, type, audioManager_);

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

	// 速度による移動と減衰
	if (velocity_.x != 0.0f || velocity_.z != 0.0f) {
		// 速度による座標の更新
		transform_.translate.x += velocity_.x * deltaTime;
		transform_.translate.z += velocity_.z * deltaTime;

		// 摩擦処理
		velocity_.x = MathUtility::Lerp(velocity_.x, 0.0f, knockBackFriction_ * deltaTime);
		velocity_.z = MathUtility::Lerp(velocity_.z, 0.0f, knockBackFriction_ * deltaTime);

		// 速度が十分に小さくなったら完全に停止させる
		if (std::abs(velocity_.x) < 0.1f && std::abs(velocity_.z) < 0.1f) {
			velocity_ = {0.0f, 0.0f, 0.0f};
		}
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

		// AI更新前の座標を記録
		Vector3 prevPos = transform_.translate;

		// AIインスタンス更新
		ai_->Update(deltaTime, player, enemyBulletManager, wallManager, doorManager, glassManager, enemyBombManager);

		// アニメーション遷移処理
		std::string targetModel = "HumanIdle.gltf"; // 待機

		if (ai_->GetState() == EnemyAI::State::Hold) {
			// 拘束時
			targetModel = "HumanIdle.gltf";
		} else if (ai_->GetState() == EnemyAI::State::Vigilance) {
			// 警戒時
			targetModel = "HumanVigilanceWalk.gltf";
		} else {
			// 通常時
			float dx = transform_.translate.x - prevPos.x;
			float dz = transform_.translate.z - prevPos.z;
			if ((dx * dx + dz * dz) > 0.0001f) {
				targetModel = "HumanNormalWalk.gltf"; // 歩き
			} else {
				targetModel = "HumanIdle.gltf"; // 待機
			}
		}

		// 現在のモデルと違う場合のみ、モデルを切り替えてアニメーション再生
		if (render_->GetFilepath() != targetModel) {
			render_->SetModel(targetModel);
			KeyframeAnimation keyframeAnim;
			Animation anim = keyframeAnim.LoadAnimationFile(targetModel);
			render_->GetObject3d()->PlayAnimation(anim);
		}

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

	Transform gunTransform;
	gunTransform.scale = {2.0f, 2.0f, 2.0f};
	float gunYaw = transform_.rotate.y - (std::numbers::pi_v<float> / 2.0f);
	gunTransform.rotate = {-std::numbers::pi_v<float> / 2.0f, gunYaw, 0.0f};

	// 手のボーン座標に追従
	gunTransform.translate = render_->GetBonePos(L"ボーン.007");

	renderGun_->Update(gunTransform);
}

void Enemy::Draw() {
	if (!isDead_) {
		// 描画
		if (isBlinkVisible_) {
			render_->Draw();
			renderGun_->Draw();
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

	// 瞬時に初速を与える
	velocity_ = {dir.x * knockBackPower_, 0.0f, dir.z * knockBackPower_};
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

void Enemy::StopKnockback() { velocity_ = {0.0f, 0.0f, 0.0f}; }

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
		ai_->Initialize(&transform_, ctx_, type_, audioManager_);
		Visionparam param = ai_->GetVisionParam();
		visionCone_->Initialize(ctx_, param.radius, param.angle);
	}
}