#pragma once
#include <memory>
#include "ParticleState.h"
#include "EngineContext.h"

// パーティクル処理モジュールの共通インターフェイス。
// Initialize: 発生時の初期化（ランダム初期値など）
// Update: 毎フレームの更新（位置・速度・色の変化など）
// PrepareRender: 描画前にインスタンシング用データを準備したいモジュール向け（任意）
class ParticleModule {
public:
    virtual ~ParticleModule() = default;

    // 発生（Spawn）時に呼ぶ。ParticleState を初期化する用途。
    virtual void Initialize(ParticleState& particle, EngineContext* ctx) {}

    // 毎フレーム呼ばれる更新処理。
	virtual void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {}

    // 描画直前に呼べるフック（WVP などレンダリング用データ準備を行う場合）
	virtual void PrepareRender(const ParticleState& particle, EngineContext* ctx) {}
};