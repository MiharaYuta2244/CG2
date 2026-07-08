#include "EffectGenerator.h"
#include "HitFlashModule.h"
#include "HitRingModule.h"
#include "HitSparkModule.h"
#include "MuzzleFlashModule.h"
#include "MuzzleSparkModule.h"
#include "MuzzleSmokeModule.h"

using namespace TinyEngine;

void EffectGenerator::CreateHitEffect(EngineContext* ctx, const Vector3& pos, std::vector<std::unique_ptr<Particle>>& container) {
	// 火花
	auto sparks = std::make_unique<Particle>();
	sparks->Initialize(ctx, pos, "white.png", std::make_unique<HitSparkModule>(), nullptr, ParticleMeshType::Square);
	sparks->SetEmitMode(false, 0.05f);
	sparks->SetEmitterParam(50, 0.01f);
	container.push_back(std::move(sparks));

	// 中心の閃光フラッシュ
	auto flash = std::make_unique<Particle>();
	flash->Initialize(ctx, pos, "AttractEffect.png", std::make_unique<HitFlashModule>(), nullptr, ParticleMeshType::Square);
	flash->SetEmitMode(false, 0.05f);
	flash->SetEmitterParam(4, 0.01f);
	container.push_back(std::move(flash));

	// 拡散する衝撃波リング
	auto ringWave = std::make_unique<Particle>();
	ringWave->Initialize(ctx, pos, "gradationLine.png", std::make_unique<HitRingModule>(), nullptr, ParticleMeshType::Cylinder);
	ringWave->SetEmitMode(false, 0.05f);
	ringWave->SetEmitterParam(1, 0.01f);
	container.push_back(std::move(ringWave));
}

void EffectGenerator::CreateMuzzleFlash(EngineContext* ctx, const Vector3& pos, const Vector3& direction, std::vector<std::unique_ptr<Particle>>& container) {
	// 閃光
	auto flash = std::make_unique<Particle>();
	flash->Initialize(ctx, pos, "AttractEffect.png", std::make_unique<MuzzleFlashModule>(), nullptr, ParticleMeshType::Square);
	flash->SetEmitMode(false, 0.05f);
	flash->SetEmitterParam(1, 0.01f);
	container.push_back(std::move(flash));

	// 火花
	auto sparks = std::make_unique<Particle>();
	sparks->Initialize(ctx, pos, "white.png", std::make_unique<MuzzleSparkModule>(direction), nullptr, ParticleMeshType::Square);
	sparks->SetEmitMode(false, 0.05f);
	sparks->SetEmitterParam(10, 0.01f);
	container.push_back(std::move(sparks));

	// 煙
	auto smoke = std::make_unique<Particle>();
	smoke->Initialize(ctx, pos, "Dust.png", std::make_unique<MuzzleSmokeModule>(direction), nullptr, ParticleMeshType::Square);
	smoke->SetEmitMode(false, 0.05f);
	smoke->SetEmitterParam(3, 0.01f);
	container.push_back(std::move(smoke));
}

void EffectGenerator::CreateEnemyDeathEffect(EngineContext* ctx, const Vector3& pos, std::list<std::unique_ptr<Particle>>& container) {
	// 火花
	auto sparks = std::make_unique<Particle>();
	sparks->Initialize(ctx, pos, "white.png", std::make_unique<HitSparkModule>(), nullptr, ParticleMeshType::Square);
	sparks->SetEmitMode(false, 0.05f);
	sparks->SetEmitterParam(50, 0.01f);
	container.push_back(std::move(sparks));

	// 中心フラッシュ
	auto flash = std::make_unique<Particle>();
	flash->Initialize(ctx, pos, "AttractEffect.png", std::make_unique<HitFlashModule>(), nullptr, ParticleMeshType::Square);
	flash->SetEmitMode(false, 0.05f);
	flash->SetEmitterParam(4, 0.01f);
	container.push_back(std::move(flash));

	// 拡散する衝撃波リング
	auto ringWave = std::make_unique<Particle>();
	ringWave->Initialize(ctx, pos, "gradationLine.png", std::make_unique<HitRingModule>(), nullptr, ParticleMeshType::Cylinder);
	ringWave->SetEmitMode(false, 0.05f);
	ringWave->SetEmitterParam(1, 0.01f);
	container.push_back(std::move(ringWave));
}