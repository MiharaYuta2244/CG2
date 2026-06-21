#include "MuzzleSparkModule.h"

void MuzzleSparkModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	float speed = RandomUtils::RangeFloat(15.0f, 30.0f);
	float spreadX = RandomUtils::RangeFloat(-0.3f, 0.3f);
	float spreadY = RandomUtils::RangeFloat(-0.3f, 0.3f);
	float spreadZ = RandomUtils::RangeFloat(-0.3f, 0.3f);

	particle.velocity = {(shootDir_.x + spreadX) * speed, (shootDir_.y + spreadY) * speed, (shootDir_.z + spreadZ) * speed};

	particle.color = {1.0f, 0.6f, 0.1f, 1.0f};
	particle.lifeTime = RandomUtils::RangeFloat(0.1f, 0.3f);
	particle.currentTime = 0.0f;

	particle.transform.scale = {0.2f, 0.2f, 0.4f};
	particle.transform.rotate.x = std::numbers::pi_v<float> / 2.0f;
}

void MuzzleSparkModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	particle.velocity.x *= 0.8f;
	particle.velocity.y *= 0.8f;
	particle.velocity.z *= 0.8f;

	float rate = 1.0f - (particle.currentTime / particle.lifeTime);
	particle.transform.scale = {0.2f * rate, 0.2f * rate, 0.4f * rate};
}