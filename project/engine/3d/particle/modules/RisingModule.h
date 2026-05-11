#pragma once
#include "ParticleModule.h"
#include "Random.h"

/// <summary>
/// 上昇するパーティクルのモジュール
/// </summary>
class RisingModule : public ParticleModule {
public:
	RisingModule(float lifeMin = 5.0f, float lifeMax = 6.0f, float riseSpeed = 1.2f, float horizontalJitter = 0.5f, float sizeMin = 2.0f, float sizeMax = 3.0f)
	    : lifeMin_(lifeMin), lifeMax_(lifeMax), riseSpeed_(riseSpeed), horizontalJitter_(horizontalJitter), sizeMin_(sizeMin), sizeMax_(sizeMax) {}

	void Initialize(ParticleState& particle, EngineContext* ctx) override;
	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	float lifeMin_;
	float lifeMax_;
	float riseSpeed_;
	float horizontalJitter_;
	float sizeMin_;
	float sizeMax_;
	const float damping_ = 0.985f;
};
