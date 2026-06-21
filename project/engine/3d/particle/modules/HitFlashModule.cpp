#include "HitFlashModule.h"

void HitFlashModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	particle.velocity = {0.0f, 0.0f, 0.0f};
	particle.color = {1.0f, 0.9f, 0.6f, 1.0f};
	particle.lifeTime = 0.15f;
	particle.currentTime = 0.0f;
	particle.transform.scale = {baseSize_, baseSize_, baseSize_};
	particle.transform.rotate.x = std::numbers::pi_v<float> / 2.0f;
}

void HitFlashModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	float progress = particle.currentTime / particle.lifeTime;
	float currentScale = baseSize_ + (progress * 4.0f);
	particle.transform.scale = {currentScale, currentScale, currentScale};
}