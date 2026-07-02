#pragma once
#include "ParticleModule.h"
#include "Random.h"

/// <summary>
/// 敵の発射前チャージエフェクト用モジュール
/// </summary>
class ChargeModule : public ParticleModule {
public:
	ChargeModule(
	    float lifeMin = 0.25f, float lifeMax = 0.45f, float burstSpeedMin = 3.0f, float burstSpeedMax = 6.0f, float sizeMin = 0.2f, float sizeMax = 0.4f, float returnStartRatio = 0.4f,
	    float pullAcceleration = 6.0f, Vector4 baseColor = {0.6f, 0.8f, 1.0f, 1.0f}, Vector4 peakColor = {0.1f, 0.4f, 1.0f, 1.0f})
	    : lifeMin_(lifeMin), lifeMax_(lifeMax), burstSpeedMin_(burstSpeedMin), burstSpeedMax_(burstSpeedMax), sizeMin_(sizeMin), sizeMax_(sizeMax), returnStartRatio_(returnStartRatio),
	      pullAcceleration_(pullAcceleration), baseColor_(baseColor), peakColor_(peakColor) {}

	void Initialize(ParticleState& particle, EngineContext* ctx) override;
	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	float lifeMin_;
	float lifeMax_;

	// 弾け飛ぶ初速の範囲
	float burstSpeedMin_;
	float burstSpeedMax_;

	// 粒の大きさの範囲
	float sizeMin_;
	float sizeMax_;

	// 寿命の何割経過した時点で中心へ戻り始めるか
	float returnStartRatio_;

	// 中心へ戻る際にどれだけ加速するか
	float pullAcceleration_;

	// 発生直後の色
	Vector4 baseColor_;

	// 中心へ戻りきる直前の色。ここへ向けて発光を強めていく
	Vector4 peakColor_;
};