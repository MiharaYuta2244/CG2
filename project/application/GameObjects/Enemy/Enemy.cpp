#include "Enemy.h"
#include "EnemyBulletManager.h"
#include "GameObjects/Stageobjects/Door/DoorManager.h"
#include "GameObjects/Stageobjects/Wall/WallManager.h"
#include "MuzzleFlashModule.h"
#include "MuzzleSmokeModule.h"
#include "MuzzleSparkModule.h"

using namespace TinyEngine;

void Enemy::Initialize(EngineContext* ctx, Vector3 pos, EnemyType type) {
	ctx_ = ctx;

	type_ = type;
	if (type_ == EnemyType::Normal) {
		hp_ = 1;
		color_ = {1, 1, 1, 1};
	} else if (type_ == EnemyType::Shotgun) {
		hp_ = 2;
		color_ = {1, 0, 0, 1};
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
}

void Enemy::Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager, DoorManager* doorManager) {
	if (isDead_ || !isMove_) {
		return;
	}

	// AIインスタンス更新
	ai_->Update(deltaTime, player, enemyBulletManager, wallManager);

	if (ai_->IsShotThisFrame()) {
		GenerateMuzzleFlash(ai_->GetShotDirection());
	}

	// プレイヤー発見時に「!」マークの生成
	if (lastState != ai_->GetState() && ai_->GetState() == EnemyAI::State::Vigilance) {
		GenerateExMark();
	}

	// 「!」マークのアニメーションが終了していれば「!」マークインスタンスを削除
	if (exclamationMark_) {
		if (!exclamationMark_->IsFinishedAnimation()) {
			exclamationMark_.reset();
			exclamationMark_ = nullptr;
		}
	}

	// AIの状態を取得して色を変える
	if (ai_->GetState() == EnemyAI::State::Vigilance) {
		// 警戒状態なら赤色
		visionCone_->SetColor({1.0f, 0.0f, 0.0f, 0.3f});
	} else {
		// 通常状態なら緑色
		visionCone_->SetColor({0.0f, 1.0f, 0.0f, 0.3f});
	}

	// 視界
	visionCone_->SetTranslate(transform_.translate);
	visionCone_->SetRotate(transform_.rotate);
	visionCone_->Update(wallManager->GetWalls(), doorManager->GetDoors());

	if (knockBackAnim_.anim.GetIsActive()) {
		knockBackAnim_.anim.Update(deltaTime, knockBackAnim_.temp);
		transform_.translate = knockBackAnim_.temp;
	}

	// 「!」マークの更新
	if (exclamationMark_) {
		exclamationMark_->Update(deltaTime, transform_.translate);
	}

	// 当たり判定更新　衝突判定用
	UpdateCollision();

	// 敵AIの状態を記録
	lastState = ai_->GetState();

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
}

void Enemy::Damage() {
	if (isDead_)
		return;

	hp_--;
	if (hp_ <= 0) {
		Kill();
	} else {
		ai_->SetState(EnemyAI::State::Normal);
		ai_->ResetShotTimer();
	}

	damageBlinkTimer_ = 1.0f; // 点滅させる時間を設定
	isBlinkVisible_ = true;   // 点滅用のフラグを立てる
}

void Enemy::UpdateCollision() {
	Vector3 pos = transform_.translate;
	bodyCol_.max = {pos.x + 0.5f, pos.y, pos.z + 0.5f};
	bodyCol_.min = {pos.x - 0.5f, pos.y, pos.z - 0.5f};
}

void Enemy::GenerateExMark() {
	// 「!」マークインスタンス生成&初期化
	exclamationMark_ = std::make_unique<ExclamationMark>();
	Vector3 pos = {transform_.translate.x, transform_.translate.y + 1.0f, transform_.translate.z};
	exclamationMark_->Initialize(ctx_, transform_.translate);
}

void Enemy::GenerateMuzzleFlash(const Vector3& direction) {
	Vector3 muzzlePos = transform_.translate + direction * 1.2f;

	// 閃光
	auto flash = std::make_unique<Particle>();
	flash->Initialize(ctx_, muzzlePos, "AttractEffect.png", std::make_unique<MuzzleFlashModule>(), nullptr, ParticleMeshType::Square);
	flash->SetEmitMode(false, 0.05f);
	flash->SetEmitterParam(1, 0.01f);
	muzzleParticles_.push_back(std::move(flash));

	// 火花
	auto sparks = std::make_unique<Particle>();
	sparks->Initialize(ctx_, muzzlePos, "white.png", std::make_unique<MuzzleSparkModule>(direction), nullptr, ParticleMeshType::Square);
	sparks->SetEmitMode(false, 0.05f);
	sparks->SetEmitterParam(10, 0.01f);
	muzzleParticles_.push_back(std::move(sparks));

	// 煙
	auto smoke = std::make_unique<Particle>();
	smoke->Initialize(ctx_, muzzlePos, "Dust.png", std::make_unique<MuzzleSmokeModule>(direction), nullptr, ParticleMeshType::Square);
	smoke->SetEmitMode(false, 0.05f);
	smoke->SetEmitterParam(3, 0.01f);
	muzzleParticles_.push_back(std::move(smoke));
}

void Enemy::SetAIState(EnemyAI::State state) {
	if (ai_) {
		ai_->SetState(state);
	}
}