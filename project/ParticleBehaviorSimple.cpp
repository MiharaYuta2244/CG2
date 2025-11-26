#include "ParticleBehaviorSimple.h"
#include "MathUtility.h"
#include "MathOperator.h"

void ParticleBehaviorSimple::InitializeParticle(ParticleState& particle, const Vector3& translate) {
	particle.transform.scale = {1.0f, 1.0f, 1.0f};
	particle.transform.rotate = {0.0f, 0.0f, 0.0f};
	// 基準位置を指定してランダムな範囲にパーティクルを生成する（既存ロジックを移植）
	Vector3 randomTranslate = {RandomUtils::RangeFloat(-1, 1), RandomUtils::RangeFloat(-1, 1), RandomUtils::RangeFloat(-1, 1)};
	particle.transform.translate = translate + randomTranslate;
	particle.velocity = {RandomUtils::RangeFloat(-1, 1), RandomUtils::RangeFloat(-1, 1), RandomUtils::RangeFloat(-1, 1)};
	particle.color = {RandomUtils::RangeFloat(0, 1), RandomUtils::RangeFloat(0, 1), RandomUtils::RangeFloat(0, 1), 1.0f};
	particle.lifeTime = RandomUtils::RangeFloat(1, 3);
	particle.currentTime = 0.0f;
}

void ParticleBehaviorSimple::UpdateParticle(ParticleState& particle, float deltaTime) {
	// 物理更新の基本
	particle.transform.translate += particle.velocity * deltaTime;
	particle.currentTime += deltaTime;
	// 色やスケールを時間で変化させたいならここで実装する
}
