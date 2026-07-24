#pragma once
#include "AABB.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "GameTimer.h"
#include "Particle.h"

/// <summary>
/// 敵の爆弾
/// </summary>
class EnemyBomb {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, Vector3 pos, Vector3 velocity, Vector3 targetPos);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// 当たり判定のGetter
	AABB GetCollision() const { return collision_; };
	AABB GetBombCollision() const { return bombCollision_; };

	// 終了フラグGetter
	bool IsFinished() const { return isFinished_; }

	// 爆発中かどうかGetter
	bool IsExploded() const { return bombTimer_.IsEnd(); }

private:
	// 当たり判定の更新
	void UpdateCollsion();
	void UpdateBombCollsion();

	// エフェクトの生成
	void GenerateBombEffect();

private:
	std::unique_ptr<ObjectRender> render_;                          // 本体描画
	std::vector<std::unique_ptr<TinyEngine::Particle>> bombEffect_; // 爆破時エフェクト
	Transform transform_;
	EngineContext* ctx_ = nullptr;
	AABB collision_;                         // 本体当たり判定
	AABB bombCollision_;                     // 爆発時の当たり判定
	GameTimer bombTimer_;                    // 爆破タイマー
	Vector3 bombScale_ = {6.0f, 6.0f, 6.0f}; // 爆発のスケール
	Vector3 velocity_ = {0.0f, 0.0f, 10.0f}; // 移動速度
	float damping_ = 3.0f;                   // 減衰率
	float speed_ = 60.0f;                    // 速度
	bool isFinished_ = false;                // 削除用フラグ
};
