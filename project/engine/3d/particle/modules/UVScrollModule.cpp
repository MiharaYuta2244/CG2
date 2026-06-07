#include "UVScrollModule.h"

void UVScrollModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	(void)ctx;

	// 定位置から動かさないため、初期速度をゼロに設定
	particle.velocity = {0.0f, 0.0f, 0.0f};

	// パラメータ設定
	particle.color = color_;
	particle.lifeTime = 5.0f;
	particle.currentTime = 0.0f;
	particle.uvScroll = scrollSpeed_;
}

void UVScrollModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	(void)ctx;

	// 速度のリセット
	particle.velocity = {0.0f, 0.0f, 0.0f};

	// UVのオフセット量を計算
	currentUVOffset_.x += scrollSpeed_.x * deltaTime;
	currentUVOffset_.y += scrollSpeed_.y * deltaTime;
}