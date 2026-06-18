#include "Enemy.h"
#include "EnemyBulletManager.h"

using namespace TinyEngine;

void Enemy::Initialize(EngineContext* ctx, Vector3 pos) {
	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = pos;

	ctx_ = ctx;

	// 描画用インスタンスの生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Hiyoko.obj");
	render_->SetTransform(transform_);
	render_->SetEnvScale(envScale_);

	// AIインスタンス生成&初期化
	ai_ = std::make_unique<EnemyAI>();
	ai_->Initialize(&transform_, ctx);

	// 視界インスタンス生成&初期化
	visionCone_ = std::make_unique<VisionCone>();
	Visionparam param = ai_->GetVisionParam();
	visionCone_->Initialize(ctx, param.radius, param.angle);
}

void Enemy::Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager) {
	if (isDead_) {
		return;
	}

	// AIインスタンス更新
	if (enableMove_) {
		ai_->Update(deltaTime, player, enemyBulletManager, wallManager);
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
	visionCone_->Update(wallManager->GetWalls());

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
		render_->Draw();

		// 視界
		if (enableMove_) {
			visionCone_->Draw();
		}

		// 「!」マークの描画
		if (exclamationMark_) {
			exclamationMark_->Draw();
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
