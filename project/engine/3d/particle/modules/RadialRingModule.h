#pragma once
#include "ParticleModule.h"
#include "Random.h"

/// <summary>
/// 放射状に広がるリング型のパーティクルモジュール
/// </summary>
class RadialRingModule : public ParticleModule {
public:
	RadialRingModule(float lifeMin = 0.7f, float lifeMax = 1.3f, float baseSpeed = 6.0f, float sizeMin = 0.18f, float sizeMax = 0.38f, float yVelocity = 0.05f)
	    : lifeMin_(lifeMin), lifeMax_(lifeMax), baseSpeed_(baseSpeed), sizeMin_(sizeMin), sizeMax_(sizeMax), yVelocity_(yVelocity) {}

	void Initialize(ParticleState& particle, EngineContext* ctx) override;
	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	float lifeMin_;
	float lifeMax_;
	float baseSpeed_;
	float sizeMin_;
	float sizeMax_;
	float yVelocity_;
	const float damping_ = 0.97f;
};
