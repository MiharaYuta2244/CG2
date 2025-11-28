#include "RisingModule.h"
#include "MathOperator.h"
#include <cmath>
#include <numbers>

void RisingModule::Initialize(ParticleState& particle, EngineContext* /*ctx*/) {
	particle.lifeTime = RandomUtils::RangeFloat(lifeMin_, lifeMax_);
	particle.currentTime = 0.0f;

	// サイズ
	float s = RandomUtils::RangeFloat(sizeMin_, sizeMax_);
	particle.transform.scale = {s, s, s};

	// 色
	float base = RandomUtils::RangeFloat(0.0f, 1.0f);
	particle.color = {base, base, base, 1.0f};

	// 初速
	particle.velocity.x = RandomUtils::RangeFloat(-horizontalJitter_, horizontalJitter_);
	particle.velocity.z = RandomUtils::RangeFloat(-horizontalJitter_, horizontalJitter_);
	particle.velocity.y = RandomUtils::RangeFloat(-horizontalJitter_, horizontalJitter_);
}

void RisingModule::Update(ParticleState& particle, float deltaTime, EngineContext* /*ctx*/) {
	// ゆっくり上昇
	particle.velocity.y = 5.0f;

	// 横方向は減衰
	particle.velocity.x *= std::pow(damping_, deltaTime * 60.0f);
	particle.velocity.z *= std::pow(damping_, deltaTime * 60.0f);

	// 経過時間による透明度変更
	if (particle.lifeTime > 0.0f) {
		float t = particle.currentTime / particle.lifeTime;
		particle.color.w = std::clamp(1.0f - t, 0.0f, 1.0f);
	}

	// わずかにサイズを縮小
	particle.transform.scale *= 0.992f;
}