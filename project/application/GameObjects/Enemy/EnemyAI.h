#pragma once
#include "AABB.h"
#include "EnemyBullet.h"
#include "GameTimer.h"
#include "Transform.h"
#include <vector>

class EnemyBulletManager;
class WallManager;
class Player;

struct Visionparam {
	float radius;
	float angle;
};

/// <summary>
/// 敵AIクラス
/// </summary>
class EnemyAI {
public:
	enum class State {
		Normal,    // 徘徊、通常状態
		Vigilance, // 警戒、プレイヤー追跡状態
	};

	// 初期化
	void Initialize(Transform* transform, EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager);

	// Getter
	State GetState() const { return state_; }
	Visionparam GetVisionParam() const { return visionParam_; }

private:
	// プレーヤー方向に回転する
	void LookatPlayer(float deltaTime, Vector3 playerPos, Vector3 enemyPos);

	// 通常状態の更新処理
	void UpdateNormal(float deltaTime, Player* player, WallManager* wallManager);

	// 警戒状態の更新処理
	void UpdateVigilance(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager);

	// プレイヤーが視界内にいるか判定する
	bool CheckPlayerInVision(Player* player, WallManager* wallManager);

	// 線分（敵からプレイヤー）とAABB（壁）の交差判定
	bool IsSegmentIntersectAABB(const Vector3& start, const Vector3& end, const AABB& aabb);

private:
	Transform* transform_ = nullptr;
	EngineContext* ctx_;

	// 敵の状態
	State state_ = State::Normal;

	// 視界のパラメータ
	Visionparam visionParam_ = {30.0f, 40.0f};
	float lostSightTimer_ = 0.0f;           // プレイヤーを見失ってからの経過時間
	const float kLostSightDuration_ = 2.0f; // 視界外でも維持する時間
	Vector3 lastKnownPlayerPos_;            // 最後に見えたプレイヤーの座標

	// 射撃用パラメータ
	float shotTimer_ = 0.0f;                         // 射撃までの残り時間タイマー
	float shotInterval_ = 2.0f;                      // 射撃間隔
	float attackRange_ = visionParam_.radius / 2.0f; // 射撃を行う距離

	// 経路探索用
	std::vector<Vector3> currentPath_; // A*で計算した経路
	int currentWaypointIndex_ = 0;     // 現在向かっている頂点
	float pathUpdateTimer_ = 0.0f;     // 経路再計算タイマー
};
