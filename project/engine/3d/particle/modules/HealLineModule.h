#pragma once
#include "ParticleModule.h"

class HealLineModule : public ParticleModule {
public:
	void Initialize(ParticleState& particle, EngineContext* ctx) override;
	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	float baseWidth_ = 0.05f;
};