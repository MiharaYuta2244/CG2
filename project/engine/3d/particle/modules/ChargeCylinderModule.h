#pragma once
#include "Gameobjects/Enemy/EnemyAI.h"
#include "ParticleModule.h"

class ChargeCylinderModule : public ParticleModule {
public:
	ChargeCylinderModule(EnemyAI* ai, Transform* enemyTransform, float maxRadius = 4.0f, float minRadius = 0.5f, float height = 2.0f);

	void Initialize(ParticleState& particle, EngineContext* ctx) override;
	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	EnemyAI* ai_;
	Transform* enemyTransform_;
	float maxRadius_;
	float minRadius_;
	float height_;
	float pulsePhase_;
};