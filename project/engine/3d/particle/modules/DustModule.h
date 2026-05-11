#pragma once
#include "ParticleModule.h"
#include "Random.h"

/// <summary>
/// ほこりパーティクルのモジュール
/// </summary>
class DustModule : public ParticleModule {
public:
    DustModule(float lifeMin = 0.4f, float lifeMax = 0.9f,
               float baseSpeed = 6.00f,
               float sizeMin = 0.15f, float sizeMax = 0.35f,
               float gravityScale = -9.8f * 0.05f)
        : lifeMin_(lifeMin),
          lifeMax_(lifeMax),
          baseSpeed_(baseSpeed),
          sizeMin_(sizeMin),
          sizeMax_(sizeMax),
          gravityScale_(gravityScale) {}

    void Initialize(ParticleState& particle, EngineContext* ctx) override;
    void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
    float lifeMin_;
    float lifeMax_;
    float baseSpeed_;
    float sizeMin_;
    float sizeMax_;
    float gravityScale_;
    const float damping_ = 0.98f;
};