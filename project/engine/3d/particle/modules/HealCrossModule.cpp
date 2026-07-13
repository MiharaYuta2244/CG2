#include "HealCrossModule.h"
#include "Random.h"

void HealCrossModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	// ゆっくり上方向へ
	float speed = RandomUtils::RangeFloat(1.0f, 2.5f);
	particle.velocity = {0.0f, speed, 0.0f};
	particle.color = {0.3f, 1.0f, 0.4f, 1.0f};
	particle.lifeTime = RandomUtils::RangeFloat(1.5f, 2.5f);
	particle.currentTime = 0.0f;

	float initialScale = baseSize_ * RandomUtils::RangeFloat(0.8f, 1.2f);
	particle.transform.scale = {initialScale, initialScale, initialScale};
	particle.transform.rotate = {std::numbers::pi_v<float> / 2.0f, 0, 0};
}

void HealCrossModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	// 寿命に合わせて徐々にフェードアウト
	float progress = particle.currentTime / particle.lifeTime;
	particle.color.w = 1.0f - progress;
}