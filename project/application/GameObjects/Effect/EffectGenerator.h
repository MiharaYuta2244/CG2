#pragma once
#include "Particle.h"
#include <list>
#include <memory>
#include <vector>

class EffectGenerator {
public:
	// ヒットエフェクトの生成
	static void CreateHitEffect(EngineContext* ctx, const Vector3& pos, std::vector<std::unique_ptr<TinyEngine::Particle>>& container);

	// マズルフラッシュの生成
	static void CreateMuzzleFlash(EngineContext* ctx, const Vector3& pos, const Vector3& direction, std::vector<std::unique_ptr<TinyEngine::Particle>>& container);

	// 敵死亡エフェクトの生成
	static void CreateEnemyDeathEffect(EngineContext* ctx, const Vector3& pos, std::list<std::unique_ptr<TinyEngine::Particle>>& container);

	// 回復エフェクト
	static void CreateHealEffect(EngineContext* ctx, const Vector3& pos, std::vector<std::unique_ptr<TinyEngine::Particle>>& container);
};