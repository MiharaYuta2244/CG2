#pragma once
#include "AABB.h"
#include "EnemyBullet.h"
#include "GameTimer.h"
#include "Transform.h"
#include <vector>

class EnemyBulletManager;
class WallManager;
class Player;

/// <summary>
/// 敵AIクラス
/// </summary>
class EnemyAI {
public:
	enum class State {
		Normal,
		Vigilance,
		Shot,
	};

	// 初期化
	void Initialize(Transform* transform, EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager);

#ifdef _DEBUG
	Vector4 GetColor() const { return color_; }
#endif

private:
	// 前方の索敵用AABBを生成する
	AABB GetSearchArea() const;

	// 敵とプレイヤーの間に壁がないか判定する
	bool CheckLineOfSight(const Vector3& start, const Vector3& end, WallManager* wallManager);

	// A*アルゴリズムにでの経路探索
	std::vector<Vector3> FindPath(const Vector3& startPos, const Vector3& targetPos, WallManager* wallManager);

	// 指定した座標が壁と重なっていないか判定
	bool IsWalkable(const Vector3& pos, WallManager* wallManager) const;

private:
	Transform* transform_ = nullptr;
	EngineContext* ctx_;

	// 敵の状態
	State state_ = State::Normal;

	// タイマー類
	GameTimer shotTimer_;       // 弾の発射タイマー
	float shotDuration_ = 2.0f; // 弾の発射間隔

	GameTimer vigilanceTimer_;
	float vigilanceDuration_ = 5.0f; // 警戒モードの持続時間

	// AI用パラメータ
	float searchDistance_ = 10.0f;         // 前方AABBの長さ
	float searchWidth_ = 4.0f;             // 前方AABBの幅
	float attackRange_ = 8.0f;             // 射撃モードに移行する距離
	float moveSpeed_ = 4.0f;               // 追跡時の移動速度
	std::vector<Vector3> currentPath_;     // 計算された経路（ウェイポイントのリスト）
	size_t currentPathIndex_ = 0;          // 現在向かっているウェイポイントのインデックス
	float pathRecalculateTimer_ = 0.0f;    // 経路再計算用タイマー
	float pathRecalculateInterval_ = 0.2f; // 再計算の間隔

#ifdef _DEBUG
	// 状態遷移確認用カラー
	Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
#endif
};
