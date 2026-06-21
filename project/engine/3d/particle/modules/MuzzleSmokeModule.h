#pragma once
#include "ParticleModule.h"
#include "Random.h"

class MuzzleSmokeModule : public ParticleModule {
private:
	Vector3 shootDir_;

public:
	MuzzleSmokeModule(const Vector3& dir) : shootDir_(dir) {}

	void Initialize(ParticleState& particle, EngineContext* ctx) override;

	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;
};
