#pragma once
#include "MathUtility.h"
#include "ParticleModule.h"

class UVScrollModule : public ParticleModule {
public:
	UVScrollModule(const Vector2& scrollSpeed, const Vector4& color) : scrollSpeed_(scrollSpeed), color_(color), currentUVOffset_({0.0f, 0.0f}) {}

	void Initialize(ParticleState& particle, EngineContext* ctx) override;
	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	Vector2 scrollSpeed_;
	Vector4 color_;
	Vector2 currentUVOffset_;
};