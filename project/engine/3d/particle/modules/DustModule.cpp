#include <numbers>
#include "DustModule.h"
#include "MathOperator.h"

void DustModule::Initialize(ParticleState& particle, EngineContext* /*ctx*/) {
    particle.lifeTime = RandomUtils::RangeFloat(lifeMin_, lifeMax_);
    particle.currentTime = 0.0f;

    // サイズ
    float s = RandomUtils::RangeFloat(sizeMin_, sizeMax_);
    particle.transform.scale = {s, s, s};

    // 色
    float g = RandomUtils::RangeFloat(0.6f, 0.85f);
    float r = g * RandomUtils::RangeFloat(0.9f, 1.0f);
    float b = g * RandomUtils::RangeFloat(0.9f, 1.0f);
    particle.color = {r, g, b, 1.0f};

    // 初速
    float angle = RandomUtils::RangeFloat(0.0f, 2.0f * std::numbers::pi_v<float>);
    float speedR = RandomUtils::RangeFloat(0.0f, baseSpeed_);
    particle.velocity.x = std::cos(angle) * speedR;
    particle.velocity.z = std::sin(angle) * speedR;
    particle.velocity.y = RandomUtils::RangeFloat(baseSpeed_ * 0.2f, baseSpeed_ * 0.6f);
}

void DustModule::Update(ParticleState& particle, float deltaTime, EngineContext* /*ctx*/) {
    // 重力
    particle.velocity.y += gravityScale_ * deltaTime;

    // 速度減衰
    particle.velocity.x *= std::pow(damping_, deltaTime * 60.0f);
    particle.velocity.y *= std::pow(damping_, deltaTime * 60.0f);
    particle.velocity.z *= std::pow(damping_, deltaTime * 60.0f);

    // 透明度を変更
    if (particle.lifeTime > 0.0f) {
        float t = particle.currentTime / particle.lifeTime;
        particle.color.w = std::clamp(1.0f - t, 0.0f, 1.0f);
    }
}