#pragma once
#include "AABB.h"
#include "EnemyBullet.h"
#include "EnemyType.h"
#include "Transform.h"
#include "GameObjects/StageObjects/Wall/WallManager.h"
#include "GameObjects/StageObjects/Door/DoorManager.h"
#include "GameObjects/StageObjects/Glass/GlassManager.h"
#include <vector>

class EnemyBulletManager;
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
		Hold,      // プレイヤーに拘束されている状態
	};

	// 初期化
	void Initialize(Transform* transform, EngineContext* ctx, EnemyType type);

	// 更新処理
	void Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager, DoorManager* doorManager, GlassManager* glassManager);

	// Getter
	State GetState() const { return state_; }
	Visionparam GetVisionParam() const { return visionParam_; }
	bool IsShotThisFrame() const { return isShotThisFrame_; }
	Vector3 GetShotDirection() const { return shotDirection_; }

	// Setter
	void SetShotHoldState(bool isShotHoldState) { isShotHoldState_ = isShotHoldState; }
	void SetState(State state) { state_ = state; }

	// ShotTimerのリセット
	void ResetShotTimer() { shotTimer_ = 0.0f; }

	// 弾が撃たれた瞬間を知らせるGetter
	float GetShotTimer() const { return shotTimer_; }
	float GetShotInterval() const { return state_ == State::Hold ? shotIntervalHold_ : shotIntervalNormal_; }

	// 撃たれた瞬間を返すGetter
	bool GetIsShot() const { return isShotThisFrame_; }

	float GetShotProgress() const {
		float interval = GetShotInterval();
		if (interval > 0.0f) {
			return std::clamp(shotTimer_ / interval, 0.0f, 1.0f);
		}
		return 0.0f;
	}

private:
	// プレーヤー方向に回転する
	void LookatPlayer(float deltaTime, Vector3 playerPos, Vector3 enemyPos, float turnSpeed = 6.0f);

	// 通常状態の更新処理
	void UpdateNormal(float deltaTime, Player* player, WallManager* wallManager, DoorManager* doorManager, GlassManager* glassManager);

	// 警戒状態の更新処理
	void UpdateVigilance(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager, DoorManager* doorManager, GlassManager* glassManager);

	// 拘束状態の更新処理
	void UpdateHold(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager);

	// プレイヤーが視界内にいるか判定する
	bool CheckPlayerInVision(Player* player, WallManager* wallManager, DoorManager* doorManager, GlassManager* glassManager);

	// 線分（敵からプレイヤー）とAABB（壁）の交差判定
	bool IsSegmentIntersectAABB(const Vector3& start, const Vector3& end, const AABB& aabb);

	// 弾の発射処理
	void Shot(Vector3 toTarget, EnemyBulletManager* enemyBulletManager);

private:
	Transform* transform_ = nullptr;
	EngineContext* ctx_;

	// 敵の状態
	State state_ = State::Normal;

	// 敵のタイプ
	EnemyType type_;

	// 視界のパラメータ
	Visionparam visionParam_ = {30.0f, 40.0f};
	float lostSightTimer_ = 0.0f;           // プレイヤーを見失ってからの経過時間
	const float kLostSightDuration_ = 2.0f; // 視界外でも維持する時間
	Vector3 lastKnownPlayerPos_;            // 最後に見えたプレイヤーの座標

	// 射撃用パラメータ
	float shotTimer_ = 0.0f;          // 射撃までの残り時間タイマー
	float shotIntervalNormal_ = 2.0f; // 射撃間隔(通常時)
	float shotIntervalHold_ = 1.5f;   // 射撃間隔(拘束時)
	float bulletMargin_ = 10.0f;      // 敵と弾の間隔
	bool isShotThisFrame_ = false;
	bool isShotHoldState_ = true;
	Vector3 shotDirection_ = {0.0f, 0.0f, 1.0f};
	float shotTimerMultiPlier_ = 0.5f; // 拘束時の発射速度を早めるための変数

	// 経路探索用
	std::vector<Vector3> currentPath_; // A*で計算した経路
	int currentWaypointIndex_ = 0;     // 現在向かっている頂点
	float pathUpdateTimer_ = 0.0f;     // 経路再計算タイマー

	// 徘徊用パラメータ
	bool isPatrolWaiting_ = true;            // 待機中かどうか
	float patrolStateTimer_ = 0.0f;          // 待機・移動の残り時間
	Vector3 patrolDir_ = {0.0f, 0.0f, 0.0f}; // 徘徊時の移動方向
	const float patrolSpeed_ = 4.0f;         // 徘徊時の移動速度

	// 聴覚用パラメータ
	float hearingRadius_ = 10.0f; // 足音が聞こえる距離
	float hearTurnSpeed_ = 8.0f;  // 足音に反応して振り向く速度
};
