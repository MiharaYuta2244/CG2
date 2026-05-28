#pragma once
#include "ParticleModule.h"

/// <summary>
/// 衝撃波パーティクルのモジュール
/// </summary>
class ShockWaveModule : public ParticleModule {
public:
    ShockWaveModule(float life = 10.0f, float startScale = 2.0f, float endScale = 4.0f)
        : life_(life), startScale_(startScale), endScale_(endScale) {}

    void Initialize(ParticleState& particle, EngineContext* ctx) override;
    void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
    float life_;
    float startScale_;
    float endScale_;
};