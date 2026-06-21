#pragma once
#include "ParticleModule.h"

class HitRingModule : public ParticleModule {
public:
	void Initialize(ParticleState& particle, EngineContext* ctx) override;

	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	float baseSize_ = 0.1f;
};