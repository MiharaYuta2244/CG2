#include "ChargeModule.h"
#include "MathOperator.h"
#include <algorithm>
#include <cmath>
#include <numbers>

void ChargeModule::Initialize(ParticleState& particle, EngineContext* /*ctx*/) {
	particle.lifeTime = RandomUtils::RangeFloat(lifeMin_, lifeMax_);
	particle.currentTime = 0.0f;

	// サイズ
	float s = RandomUtils::RangeFloat(sizeMin_, sizeMax_);
	particle.transform.scale = {s, s, s};
	particle.transform.rotate = {std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f};

	// 開始時の色
	particle.color = baseColor_;

	// 中心から外側へ弾け飛ぶ
	float angle = RandomUtils::RangeFloat(0.0f, 2.0f * std::numbers::pi_v<float>);
	float speed = RandomUtils::RangeFloat(burstSpeedMin_, burstSpeedMax_);
	particle.velocity.x = std::cos(angle) * speed;
	particle.velocity.z = std::sin(angle) * speed;
	particle.velocity.y = RandomUtils::RangeFloat(-speed * 0.2f, speed * 0.2f);
}

void ChargeModule::Update(ParticleState& particle, float deltaTime, EngineContext* /*ctx*/) {
	if (particle.lifeTime <= 0.0f) {
		return;
	}

	float t = std::clamp(particle.currentTime / particle.lifeTime, 0.0f, 1.0f);
	float tPrev = std::clamp((particle.currentTime - deltaTime) / particle.lifeTime, 0.0f, 1.0f);

	// 寿命がreturnStartRatio_を超えた瞬間に、外向きの速度を反転させて中心へ吸い込まれる動きへ切り替える
	if (tPrev < returnStartRatio_ && t >= returnStartRatio_) {
		particle.velocity.x *= -1.0f;
		particle.velocity.y *= -1.0f;
		particle.velocity.z *= -1.0f;
	}

	if (t >= returnStartRatio_) {
		// 中心へ戻る最中は、吸い込まれるように少しずつ加速させる
		float pullFactor = 1.0f + pullAcceleration_ * deltaTime;
		particle.velocity.x *= pullFactor;
		particle.velocity.y *= pullFactor;
		particle.velocity.z *= pullFactor;

		// 収束していくにつれて粒を小さくしていく
		float progress = (t - returnStartRatio_) / (1.0f - returnStartRatio_);
		float shrink = std::clamp(1.0f - progress, 0.0f, 1.0f);
		float size = sizeMax_ * shrink + 0.02f;
		particle.transform.scale = {size, size, size};

		// 中心に近づくほど発射直前の色へ寄せて発光を強める
		float colorT = std::clamp(1.0f - shrink, 0.0f, 1.0f);
		particle.color.x = baseColor_.x + (peakColor_.x - baseColor_.x) * colorT;
		particle.color.y = baseColor_.y + (peakColor_.y - baseColor_.y) * colorT;
		particle.color.z = baseColor_.z + (peakColor_.z - baseColor_.z) * colorT;
	}

	// 寿命の終わりにかけてフェードさせる
	particle.color.w = std::clamp(1.0f - t * 0.3f, 0.0f, 1.0f);
}