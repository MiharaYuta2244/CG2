#pragma once
#include "ParticleModule.h"
#include "Random.h"

using namespace TinyEngine;

class MuzzleFlashModule : public ParticleModule {
public:
	void Initialize(ParticleState& particle, EngineContext* ctx) override;

	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;
};