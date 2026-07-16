#pragma once
#include "BaseScene.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "Particle.h"
#include "GPUParticle.h"

class TestScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	std::unique_ptr<GPUParticle> gpuParticle_;
	PerView perView_;
};
