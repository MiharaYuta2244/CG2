#include "ShockWaveModule.h"
#include "Random.h"
#include <algorithm>

void ShockWaveModule::Initialize(ParticleState& particle, EngineContext* /*ctx*/) {
    particle.lifeTime = life_;
    particle.currentTime = 0.0f;
    particle.transform.scale = {startScale_, startScale_, startScale_};
    particle.velocity = {0.0f, 0.0f, 0.0f};
    particle.color = {1.0f, 0.4f, 0.1f, 1.0f};
}

void ShockWaveModule::Update(ParticleState& particle, float /*deltaTime*/, EngineContext* /*ctx*/) {
    if (particle.lifeTime <= 0.0f) return;
    float t = particle.currentTime / particle.lifeTime;
    t = std::clamp(t, 0.0f, 1.0f);
    float s = startScale_ + (endScale_ - startScale_) * t;
    particle.transform.scale = {s, s, s};
    particle.color.w = 1.0f - t;
}