#pragma once
#include "ParticleModule.h"

using namespace TinyEngine;

class HitSparkModule : public ParticleModule {
public:
	void Initialize(ParticleState& particle, EngineContext* ctx) override;

	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	float baseSize_ = 1.0f;
};