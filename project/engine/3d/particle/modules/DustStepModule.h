#pragma once
#include "ParticleModule.h"

class DustStepModule : public ParticleModule {
public:
	void Initialize(ParticleState& particle, EngineContext* ctx) override;
	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	float baseSize_ = 0.15f; // 初期サイズ
	float maxScale_ = 1.0f;  // 最大サイズ
};
