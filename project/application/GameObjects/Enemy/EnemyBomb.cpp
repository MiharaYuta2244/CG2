#include "EnemyBomb.h"
#include "GameObjects/Effect/EffectGenerator.h"

void EnemyBomb::Initialize(EngineContext* ctx, Vector3 pos, Vector3 velocity) {
	ctx_ = ctx;
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "cube.obj");
	transform_.scale = {1, 1, 1};
	transform_.rotate = {0, 0, 0};
	transform_.translate = pos;

	// 爆破タイマーの初期化
	bombTimer_.Initialize(1.0f);
}

void EnemyBomb::Update(float deltaTime) {
	// 移動処理
	transform_.translate.x += velocity_.x * deltaTime;
	transform_.translate.z += velocity_.z * deltaTime;

	// 摩擦処理
	velocity_.x -= velocity_.x * damping_ * deltaTime;
	velocity_.z -= velocity_.z * damping_ * deltaTime;

	// 爆破タイマーの更新
	if (!isFinished_) {
		bombTimer_.Update(deltaTime);
	}

	// 爆破タイマーが終了したら爆破処理へ
	if (bombTimer_.IsEnd()) {
		// 当たり判定を更新
		UpdateBombCollsion();

		// 爆発エフェクトの生成
		GenerateBombEffect();
	} else {
		// 当たり判定の更新
		UpdateCollsion();
	}

	// 爆発エフェクトの更新
	for (auto& particle : bombEffect_) {
		particle->SetTranslate(transform_.translate);
		particle->Update();
	}

	// 爆発エフェクト削除
	std::erase_if(bombEffect_, [this](const std::unique_ptr<TinyEngine::Particle>& p) {
		if (p->IsFinished()) {
			// 終了フラグを立てる
			isFinished_ = true;

			// タイマー初期化
			bombTimer_.Initialize(1.0f);
		}
		return p->IsFinished();
	});

	// 更新
	render_->Update();
}

void EnemyBomb::Draw() {
	if (!bombTimer_.IsEnd()) {
		// 本体描画
		render_->Draw();
	}

	// 爆発エフェクトの描画
	for (auto& particle : bombEffect_) {
		particle->Draw();
	}
}

void EnemyBomb::UpdateCollsion() {
	Vector3 pos = transform_.translate;
	Vector3 scale = transform_.scale;
	collision_.min = {pos.x - (scale.x / 2.0f), pos.y - (scale.y / 2.0f), pos.z - (scale.z / 2.0f)};
	collision_.min = {pos.x + (scale.x / 2.0f), pos.y + (scale.y / 2.0f), pos.z + (scale.z / 2.0f)};
}

void EnemyBomb::UpdateBombCollsion() {
	Vector3 pos = transform_.translate;
	collision_.min = {pos.x - (bombScale_.x / 2.0f), pos.y - (bombScale_.y / 2.0f), pos.z - (bombScale_.z / 2.0f)};
	collision_.min = {pos.x + (bombScale_.x / 2.0f), pos.y + (bombScale_.y / 2.0f), pos.z + (bombScale_.z / 2.0f)};
}

void EnemyBomb::GenerateBombEffect() {
	// エフェクトの生成
	EffectGenerator::CreateHitEffect(ctx_, transform_.translate, bombEffect_);
}