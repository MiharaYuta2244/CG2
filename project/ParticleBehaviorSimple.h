#pragma once
#include "IParticleBehavior.h"
#include "Random.h"

class ParticleBehaviorSimple : public IParticleBehavior{
public:
	ParticleBehaviorSimple() = default;
	~ParticleBehaviorSimple() override = default;

	void InitializeParticle(ParticleState& particle, const Vector3& translate) override;
	void UpdateParticle(ParticleState& particle, float deltaTime) override;
	std::unique_ptr<IParticleBehavior> Clone() const override { return std::make_unique<ParticleBehaviorSimple>(*this); }
};
