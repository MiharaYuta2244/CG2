#include "HealSphereModule.h"
#include "Random.h"

void HealSphereModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	float speedY = RandomUtils::RangeFloat(2.0f, 4.0f);
	float spreadX = RandomUtils::RangeFloat(-0.8f, 0.8f);
	float spreadZ = RandomUtils::RangeFloat(-0.8f, 0.8f);

	particle.velocity = {spreadX, speedY, spreadZ};
	particle.color = {0.6f, 1.0f, 0.6f, 1.0f};
	particle.lifeTime = RandomUtils::RangeFloat(1.0f, 2.0f);
	particle.currentTime = 0.0f;
	particle.transform.scale = {baseSize_, baseSize_, baseSize_};
	particle.transform.rotate = {std::numbers::pi_v<float> / 2.0f, 0, 0};
}

void HealSphereModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	// 減速させながら、サイズを小さくしていく
	particle.velocity.x *= 0.95f;
	particle.velocity.z *= 0.95f;

	float rate = 1.0f - (particle.currentTime / particle.lifeTime);
	particle.transform.scale = {baseSize_ * rate, baseSize_ * rate, baseSize_ * rate};
	particle.color.w = rate;
}