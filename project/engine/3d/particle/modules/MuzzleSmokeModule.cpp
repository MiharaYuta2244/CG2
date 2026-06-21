#include "MuzzleSmokeModule.h"

void MuzzleSmokeModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	float speed = RandomUtils::RangeFloat(2.0f, 5.0f);

	particle.velocity = {
	    shootDir_.x * speed + RandomUtils::RangeFloat(-1.0f, 1.0f), shootDir_.y * speed + RandomUtils::RangeFloat(1.0f, 3.0f), shootDir_.z * speed + RandomUtils::RangeFloat(-1.0f, 1.0f)};

	particle.color = {0.5f, 0.5f, 0.5f, 0.6f};
	particle.lifeTime = RandomUtils::RangeFloat(0.4f, 0.7f);
	particle.currentTime = 0.0f;
	particle.transform.scale = {0.3f, 0.3f, 0.3f};
	particle.transform.rotate.x = std::numbers::pi_v<float> / 2.0f;
}

void MuzzleSmokeModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	particle.velocity.x *= 0.95f;
	particle.velocity.y += 1.0f * deltaTime;
	particle.velocity.z *= 0.95f;

	float progress = particle.currentTime / particle.lifeTime;
	float currentScale = 0.3f + (progress * 1.5f);
	particle.transform.scale = {currentScale, currentScale, currentScale};
}