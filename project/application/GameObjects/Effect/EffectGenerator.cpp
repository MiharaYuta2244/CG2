#include "EffectGenerator.h"
#include "ParticleModuleHeader.h"

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
	ringWave->Initialize(ctx, pos, "gradationLine.png", std::make_unique<HitRingModule>(Vector4(1.0f, 0.3f, 0.0f, 0.8f), 5.0f), nullptr, ParticleMeshType::Cylinder);
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
	ringWave->Initialize(ctx, pos, "gradationLine.png", std::make_unique<HitRingModule>(Vector4(1.0f, 0.3f, 0.0f, 0.8f), 5.0f), nullptr, ParticleMeshType::Cylinder);
	ringWave->SetEmitMode(false, 0.05f);
	ringWave->SetEmitterParam(1, 0.01f);
	container.push_back(std::move(ringWave));
}

void EffectGenerator::CreateHealEffect(EngineContext* ctx, const Vector3& pos, std::vector<std::unique_ptr<TinyEngine::Particle>>& container){
	// 上昇する十字パーティクル
	auto cross = std::make_unique<Particle>();
	cross->Initialize(ctx, pos, "Cross.png", std::make_unique<HealCrossModule>(), nullptr, ParticleMeshType::Square);
	cross->SetEmitMode(false, 0.05f);
	cross->SetEmitterParam(15, 0.01f);
	container.push_back(std::move(cross));

	// 上昇するスフィアパーティクル
	auto sphere = std::make_unique<Particle>();
	sphere->Initialize(ctx, pos, "white.png", std::make_unique<HealSphereModule>(), nullptr, ParticleMeshType::Square);
	sphere->SetEmitMode(false, 0.05f);
	sphere->SetEmitterParam(30, 0.01f);
	container.push_back(std::move(sphere));

	// 上昇するスラッシュパーティクル
	auto slash = std::make_unique<Particle>();
	slash->Initialize(ctx, pos, "white.png", std::make_unique<HealLineModule>(), nullptr, ParticleMeshType::Square);
	slash->SetEmitMode(false, 0.05f);
	slash->SetEmitterParam(10, 0.01f);
	container.push_back(std::move(slash));

	// 地面に配置するリング
	auto ringWave = std::make_unique<Particle>();
	ringWave->Initialize(ctx, pos, "gradationLine.png", std::make_unique<HitRingModule>(Vector4(0.8f, 1.0f, 0.8f, 1.0f), 3.0f), nullptr, ParticleMeshType::Cylinder);
	ringWave->SetEmitMode(false, 0.05f);
	ringWave->SetEmitterParam(1, 0.0f);
	container.push_back(std::move(ringWave));
}