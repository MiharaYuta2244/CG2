#include "HitRingModule.h"

void HitRingModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	particle.velocity = {0.0f, 0.0f, 0.0f};
	particle.color = {1.0f, 0.3f, 0.0f, 0.8f};
	particle.lifeTime = 0.3f;
	particle.currentTime = 0.0f;
	particle.transform.scale = {baseSize_, baseSize_, baseSize_};
}

void HitRingModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	float progress = particle.currentTime / particle.lifeTime;
	float currentScale = baseSize_ + (progress * 5.0f);
	particle.transform.scale = {currentScale, currentScale, currentScale};
}