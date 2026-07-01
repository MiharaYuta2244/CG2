#pragma once
#include "AABB.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "Rect.h"
#include "OBB.h"
#include "VisionCone.h"

/// <summary>
/// 敵の弾クラス
/// </summary>
class EnemyBullet {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, Vector2 dir, Vector3 pos);

	// 更新処理
	void Update(float deltaTime, float bulletSpeed);

	// 描画処理
	void Draw();

	// 削除用の関数
	bool IsDead(const Rect<float>& rect) const;

	// 座標のGetter
	Vector3 GetPosition() const { return transform_.translate; }

	// 当たり判定Getter
	OBB GetCollision() const { return col_; }

private:
	Transform transform_;

	// 描画用インスタンス
	std::unique_ptr<TinyEngine::VisionCone> render_;

	// 方向
	Vector2 direction_;

	// AABB当たり判定
	OBB col_;

	// 生存時間管理用の変数
	float deathTimer_ = 0.0f;      // 経過時間タイマー
	const float kLifeTime_ = 0.1f; // 消滅までの時間

	// 敵と弾の余白
	float margin_ = 6.5f;
};
