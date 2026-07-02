#include "ChargeCylinderModule.h"
#include <algorithm>
#include <cmath>

ChargeCylinderModule::ChargeCylinderModule(EnemyAI* ai, Transform* enemyTransform, float maxRadius, float minRadius, float height)
    : ai_(ai), enemyTransform_(enemyTransform), maxRadius_(maxRadius), minRadius_(minRadius), height_(height), pulsePhase_(0.0f) {}

void ChargeCylinderModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	particle.lifeTime = 9999.0f;
	particle.currentTime = 0.0f;

	// 初期スケール
	particle.transform.scale = {maxRadius_, height_, maxRadius_};

	// パーティクルの色
	particle.color = {0.6f, 0.8f, 1.0f, 1.0f};
}

void ChargeCylinderModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	if (!ai_ || !enemyTransform_)
		return;

	// 敵の座標に追従させる
	particle.transform.translate = enemyTransform_->translate;

	// 射撃タイマーから発射までの割合を計算
	float timer = ai_->GetShotTimer();
	float interval = ai_->GetShotInterval();
	float ratio = 1.0f;
	if (interval > 0.0f) {
		ratio = std::clamp(timer / interval, 0.0f, 1.0f);
	}

	// 発射が近づくほど、アニメーションスピードを速くする
	float minSpeed = 1.0f; // チャージ開始時のスピード
	float maxSpeed = 6.0f; // 発射直前のスピード
	float currentSpeed = maxSpeed - (maxSpeed - minSpeed) * ratio;

	// アニメーションの進行度を更新してループさせる
	pulsePhase_ += deltaTime * currentSpeed;
	float t = std::fmod(pulsePhase_, 1.0f);

	// スケールの適用
	float currentRadius = maxRadius_ + (minRadius_ - maxRadius_) * t;
	particle.transform.scale.x = currentRadius;
	particle.transform.scale.z = currentRadius;
	particle.transform.scale.y = height_;

	// アルファ値の調整
	particle.color.w = 0.8f * (1.0f - t);
}