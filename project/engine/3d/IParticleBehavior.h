#pragma once
#include "Particle.h"
#include "ParticleState.h"
#include "Vector3.h"

struct Emitter;

class IParticleBehavior {
public:
	virtual ~IParticleBehavior() = default;

	virtual void InitializeParticle(ParticleState& particle, const Vector3& translate) = 0;

	virtual void UpdateParticle(ParticleState& particle, float deltaTime) = 0;

	virtual std::unique_ptr<IParticleBehavior> Clone() const = 0;
};