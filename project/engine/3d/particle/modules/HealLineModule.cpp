#include "HealLineModule.h"
#include "Random.h"

void HealLineModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	// かなり速い速度で上へ
	float speed = RandomUtils::RangeFloat(5.0f, 8.0f);
	particle.velocity = {0.0f, speed, 0.0f};
	particle.color = {0.5f, 1.0f, 0.2f, 0.8f};
	particle.lifeTime = RandomUtils::RangeFloat(0.5f, 1.2f);
	particle.currentTime = 0.0f;

	// Y軸方向に長く引き伸ばす
	float lengthY = RandomUtils::RangeFloat(1.5f, 4.0f);
	particle.transform.scale = {baseWidth_, lengthY, baseWidth_};
}

void HealLineModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	float progress = particle.currentTime / particle.lifeTime;
	// フェードアウト
	particle.color.w = 0.8f * (1.0f - progress);
}