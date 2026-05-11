#include "RadialRingModule.h"
#include "MathOperator.h"
#include <cmath>
#include <numbers>

void RadialRingModule::Initialize(ParticleState& particle, EngineContext* /*ctx*/) {
	particle.lifeTime = RandomUtils::RangeFloat(lifeMin_, lifeMax_);
	particle.currentTime = 0.0f;

	// サイズ
	float s = RandomUtils::RangeFloat(sizeMin_, sizeMax_);
	particle.transform.scale = {s, s, s};

	// 色
	float r = RandomUtils::RangeFloat(0.85f, 1.0f);
	float g = RandomUtils::RangeFloat(0.0f, 0.5f);
	float b = RandomUtils::RangeFloat(0.0f, 0.5f);
	particle.color = {r, g, b, 1.0f};

	// 角度を決めて円に広がる方向に発射
	float angle = RandomUtils::RangeFloat(0.0f, 2.0f * std::numbers::pi_v<float>);
	float speed = RandomUtils::RangeFloat(baseSpeed_ * 0.7f, baseSpeed_);
	particle.velocity.x = std::cos(angle) * speed;
	particle.velocity.z = std::sin(angle) * speed;
	particle.velocity.y = yVelocity_;
}

void RadialRingModule::Update(ParticleState& particle, float deltaTime, EngineContext* /*ctx*/) {
	// 水平速度に減衰
	particle.velocity.x *= std::pow(damping_, deltaTime * 60.0f);
	particle.velocity.z *= std::pow(damping_, deltaTime * 60.0f);

	// Y方向は穏やかに漂う
	particle.velocity.y = yVelocity_;

	// フェードアウト
	if (particle.lifeTime > 0.0f) {
		float t = particle.currentTime / particle.lifeTime;
		particle.color.w = std::clamp(1.0f - t, 0.0f, 1.0f);
	}

	// サイズを少し縮小
	particle.transform.scale *= 0.995f;
}
