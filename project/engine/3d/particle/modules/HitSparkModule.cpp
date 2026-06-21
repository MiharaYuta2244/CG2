#include "HitSparkModule.h"
#include "Random.h"

void HitSparkModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	float speed = RandomUtils::RangeFloat(6.0f, 14.0f);
	float theta = RandomUtils::RangeFloat(0.0f, 2.0f * std::numbers::pi_v<float>);
	float phi = RandomUtils::RangeFloat(0.0f, std::numbers::pi_v<float>);

	particle.velocity = {speed * std::sin(phi) * std::cos(theta), speed * std::cos(phi), speed * std::sin(phi) * std::sin(theta)};
	particle.color = {1.0f, RandomUtils::RangeFloat(0.4f, 0.8f), 0.1f, 1.0f};
	particle.lifeTime = RandomUtils::RangeFloat(0.25f, 0.5f);
	particle.currentTime = 0.0f;
	particle.transform.scale = {baseSize_, baseSize_, baseSize_};
	particle.transform.rotate.x = std::numbers::pi_v<float> / 2.0f;
}

void HitSparkModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	particle.velocity.x *= 0.88f;
	particle.velocity.y *= 0.88f;
	particle.velocity.z *= 0.88f;

	float rate = 1.0f - (particle.currentTime / particle.lifeTime);
	particle.transform.scale = {baseSize_ * rate, baseSize_ * rate, baseSize_ * rate};
}