#include "MuzzleFlashModule.h"

void MuzzleFlashModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	particle.velocity = {0.0f, 0.0f, 0.0f};
	particle.color = {1.0f, 0.9f, 0.4f, 1.0f};
	particle.lifeTime = 0.08f;
	particle.currentTime = 0.0f;
	particle.transform.scale = {2.0f, 2.0f, 2.0f};
	particle.transform.rotate.z = RandomUtils::RangeFloat(0.0f, 3.14159f * 2.0f);
	particle.transform.rotate.x = std::numbers::pi_v<float> / 2.0f;
}

void MuzzleFlashModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	float rate = 1.0f - (particle.currentTime / particle.lifeTime);
	particle.transform.scale = {2.0f * rate, 2.0f * rate, 2.0f * rate};
}