#include "DustStepModule.h"

void DustStepModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	particle.velocity = {0.0f, 0.0f, 0.0f};     // 砂埃は移動しない
	particle.color = {0.8f, 0.75f, 0.6f, 1.0f}; // 砂っぽい色
	particle.lifeTime = 0.4f;                   // 寿命は短め
	particle.currentTime = 0.0f;

	particle.transform.scale = {baseSize_, baseSize_, baseSize_};

	// 必要なら回転を固定
	particle.transform.rotate.x = std::numbers::pi_v<float> / 2.0f;
}

void DustStepModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	float progress = particle.currentTime / particle.lifeTime;

	// スケールは徐々に大きくなる
	float currentScale = baseSize_ + (maxScale_ - baseSize_) * progress;
	particle.transform.scale = {currentScale, currentScale, currentScale};

	// アルファは徐々に 1 → 0 へ
	float alpha = 1.0f - progress;
	particle.color.w = alpha;
}
