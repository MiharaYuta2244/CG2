#pragma once
#include "ParticleModule.h"

/// <summary>
/// 衝撃波パーティクルのモジュール
/// </summary>
class ShockWaveModule : public ParticleModule {
public:
    ShockWaveModule(float life = 0.4f, float startScale = 0.5f, float endScale = 2.0f)
        : life_(life), startScale_(startScale), endScale_(endScale) {}

    void Initialize(ParticleState& particle, EngineContext* ctx) override;
    void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
    float life_;
    float startScale_;
    float endScale_;
};