#include "EnemyAI.h"
#include "AStarPathfinder.h"
#include "EnemyBulletManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/StageObjects/Wall/WallManager.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "Random.h"
#include <cmath>

void EnemyAI::Initialize(Transform* transform, EngineContext* ctx) {
	transform_ = transform;
	ctx_ = ctx;
}

void EnemyAI::Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager) {
	// 状態に応じたUpdateを呼ぶ
	switch (state_) {
	case State::Normal:
		UpdateNormal(deltaTime, player, wallManager);
		break;
	case State::Vigilance:
		UpdateVigilance(deltaTime, player, enemyBulletManager, wallManager);
		break;
	}
}

void EnemyAI::LookatPlayer(float deltaTime, Vector3 playerPos, Vector3 enemyPos, float turnSpeed) {
	Vector3 toPlayer = playerPos - enemyPos;
	toPlayer.y = 0.0f;

	if (MathUtility::LengthSquared(toPlayer) > 0.0001f) {
		toPlayer = MathUtility::Normalize(toPlayer);

		float angleY = std::atan2(toPlayer.x, toPlayer.z);

		float current = transform_->rotate.y;
		float target = angleY;

		transform_->rotate.y = MathUtility::LerpAngle(current, target, deltaTime * turnSpeed);
	}
}

void EnemyAI::UpdateNormal(float deltaTime, Player* player, WallManager* wallManager) {
	// 視界チェックを行い、見つけたら警戒状態へ遷移
	if (CheckPlayerInVision(player, wallManager)) {
		state_ = State::Vigilance;
	}

	// 聴覚チェック
	Vector3 playerPos = player->GetPosition();
	Vector3 enemyPos = transform_->translate;
	Vector3 toPlayer = playerPos - enemyPos;
	toPlayer.y = 0.0f;
	float distSq = toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z;

	// プレイヤーが移動しているかどうか
	bool isPlayerMoving = player->IsMoving();

	// プレイヤーが動いていて、かつ音が聞こえる範囲内にいる場合
	if (isPlayerMoving && distSq <= hearingRadius_ * hearingRadius_) {
		// 音のした方向へ素早く振り向く
		LookatPlayer(deltaTime, playerPos, enemyPos, hearTurnSpeed_);

		// 音に気を取られている演出として、徘徊状態を待機で上書きする
		isPatrolWaiting_ = true;
		patrolStateTimer_ = 0.5f; // 少しの間、音の方向を警戒して立ち止まる

		return; // 音に反応している間は通常の徘徊移動を行わないようにする
	}

	// 徘徊処理
	patrolStateTimer_ -= deltaTime;
	if (isPatrolWaiting_) {
		// 待機中
		if (patrolStateTimer_ <= 0.0f) {
			// 待機終了から移動開始
			isPatrolWaiting_ = false;

			// 次の移動時間
			patrolStateTimer_ = RandomUtils::RangeFloat(2.0f, 4.0f);

			// ランダム角度
			float pi = std::numbers::pi_v<float>;
			float randomAngle = RandomUtils::RangeFloat(0.0f, 360.0f) * (pi / (pi / 2.0f));

			patrolDir_.x = std::sin(randomAngle);
			patrolDir_.y = 0.0f;
			patrolDir_.z = std::cos(randomAngle);
		}
	} else {
		// 移動中
		if (patrolStateTimer_ <= 0.0f) {
			// 移動終了から待機開始
			isPatrolWaiting_ = true;
			patrolStateTimer_ = RandomUtils::RangeFloat(1.0f, 3.0f);
		} else {

			// 壁チェック
			Vector3 checkPos = transform_->translate;
			float checkDistance = 1.5f;
			checkPos.x += patrolDir_.x * checkDistance;
			checkPos.z += patrolDir_.z * checkDistance;

			bool isHitWall = false;
			const auto& walls = wallManager->GetWalls();
			for (const auto& wall : walls) {
				if (IsSegmentIntersectAABB(transform_->translate, checkPos, wall->GetCollision())) {
					isHitWall = true;
					break;
				}
			}

			if (isHitWall) {
				// 壁にぶつかりそうになったら短い待機へ
				isPatrolWaiting_ = true;
				patrolStateTimer_ = 0.5f;

			} else {
				// 壁が無ければ移動
				transform_->translate.x += patrolDir_.x * patrolSpeed_ * deltaTime;
				transform_->translate.z += patrolDir_.z * patrolSpeed_ * deltaTime;

				// 進行方向へ体を向ける
				LookatPlayer(deltaTime, transform_->translate + patrolDir_, transform_->translate);
			}
		}
	}
}

void EnemyAI::UpdateVigilance(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager) {
	Vector3 playerPos = player->GetPosition();
	Vector3 enemyPos = transform_->translate;

	// 現在プレイヤーが見えているかチェック
	bool canSeePlayer = CheckPlayerInVision(player, wallManager);

	if (canSeePlayer) {
		// 見えている場合、タイマーをリセットし、最後の位置を更新
		lostSightTimer_ = kLostSightDuration_;
		lastKnownPlayerPos_ = playerPos;

		// プレイヤーの方を向く
		LookatPlayer(deltaTime, playerPos, enemyPos);

		// 追跡経路が残っていたらクリア
		if (!currentPath_.empty()) {
			currentPath_.clear();
			pathUpdateTimer_ = 0.0f;
		}

		// 警戒状態時は基本的に射撃を行う
		Vector3 toTarget = playerPos - enemyPos;
		toTarget.y = 0.0f;

		shotTimer_ += deltaTime;
		if (shotTimer_ >= shotInterval_) {
			auto bullet = std::make_unique<EnemyBullet>();
			Vector3 dir3D = MathUtility::Normalize(toTarget);
			Vector2 dir2D = {dir3D.x, dir3D.z};

			Vector3 spawnPos = enemyPos;
			spawnPos.x += dir3D.x * bulletMargin_;
			spawnPos.z += dir3D.z * bulletMargin_;
			spawnPos.y += 1.0f;

			bullet->Initialize(ctx_, dir2D, spawnPos);
			enemyBulletManager->AddBullet(std::move(bullet));
			shotTimer_ = 0.0f;
		}
	} else {
		// 見失った場合、タイマーを減らす
		lostSightTimer_ -= deltaTime;

		// 一定時間見つからなかったら完全に諦めて通常状態に戻る
		if (lostSightTimer_ <= 0.0f) {
			state_ = State::Normal;
			currentPath_.clear(); // 経路もリセット
			shotTimer_ = 0.0f;    // 弾のクールタイムリセット
			return;               // ここで初めて追跡を終了する
		}

		// 目標を「最後に見えた位置」にして経路計算し、A*で回り込む
		pathUpdateTimer_ -= deltaTime;

		if (pathUpdateTimer_ <= 0.0f) {
			currentPath_ = AStarPathfinder::FindPath(enemyPos, lastKnownPlayerPos_, wallManager);
			currentWaypointIndex_ = 0;
			pathUpdateTimer_ = 0.5f; // 再計算の頻度
		}

		// 計算された経路に沿って移動
		if (!currentPath_.empty() && currentWaypointIndex_ < currentPath_.size()) {
			Vector3 targetWaypoint = currentPath_[currentWaypointIndex_];
			Vector3 toWaypoint = targetWaypoint - enemyPos;
			toWaypoint.y = 0.0f;

			if (toWaypoint.x * toWaypoint.x + toWaypoint.z * toWaypoint.z < 1.0f) {
				// ウェイポイントに到着したら次のポイントへ
				currentWaypointIndex_++;
			} else {
				// 次のポイントへ向かって移動
				Vector3 moveDir = MathUtility::Normalize(toWaypoint);
				float moveSpeed = 10.0f;
				transform_->translate.x += moveDir.x * moveSpeed * deltaTime;
				transform_->translate.z += moveDir.z * moveSpeed * deltaTime;

				// 見失って移動している間は進行方向を向かせる
				LookatPlayer(deltaTime, enemyPos + moveDir, enemyPos);
			}
		}
	}

	// 追跡の目標を、最後に見えた座標にする
	Vector3 toTarget = lastKnownPlayerPos_ - enemyPos;
	toTarget.y = 0.0f;
	float distSq = toTarget.x * toTarget.x + toTarget.z * toTarget.z;
}

bool EnemyAI::CheckPlayerInVision(Player* player, WallManager* wallManager) {
	Vector3 playerPos = player->GetPosition();
	Vector3 enemyPos = transform_->translate;

	// 高さを無視した2D平面上で計算
	Vector3 toPlayer = playerPos - enemyPos;
	toPlayer.y = 0.0f;

	// 距離判定
	float distSq = toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z;
	if (distSq > visionParam_.radius * visionParam_.radius) {
		return false; // 視界より遠い
	}

	// 角度判定
	toPlayer = MathUtility::Normalize(toPlayer);

	// 敵のY軸の回転角から、正面ベクトルを算出
	Vector3 forward;
	forward.x = std::sin(transform_->rotate.y);
	forward.y = 0.0f;
	forward.z = std::cos(transform_->rotate.y);

	// 内積を計算
	float dot = forward.x * toPlayer.x + forward.z * toPlayer.z;

	// 視野角の半分の角度のコサイン値を求める
	float halfAngleRadian = MathUtility::DegreeToRadian(visionParam_.angle / 2.0f);
	float cosLimit = std::cos(halfAngleRadian);

	if (dot < cosLimit) {
		return false; // 角度の外にいる
	}

	// 遮蔽物判定（レイキャスト / 線分とAABB）
	// 壁マネージャーから全ての壁を取得し、間に壁がないかチェック
	const auto& walls = wallManager->GetWalls();
	for (const auto& wall : walls) {
		AABB wallCol = wall->GetCollision();
		if (IsSegmentIntersectAABB(enemyPos, playerPos, wallCol)) {
			return false; // 壁に遮られている
		}
	}

	// 距離、角度、遮蔽物のすべてをクリアしたら視界に入った判定
	return true;
}

bool EnemyAI::IsSegmentIntersectAABB(const Vector3& start, const Vector3& end, const AABB& aabb) {
	// スラブ法を用いた2D線分とAABBの交差判定
	Vector3 dir = end - start;
	float tMin = 0.0f;
	float tMax = 1.0f;

	// X軸のチェック
	if (std::abs(dir.x) < 0.00001f) {
		// X方向に動いていない場合、AABBのX幅の範囲外なら絶対に当たらない
		if (start.x < aabb.min.x || start.x > aabb.max.x)
			return false;
	} else {
		float t1 = (aabb.min.x - start.x) / dir.x;
		float t2 = (aabb.max.x - start.x) / dir.x;
		if (t1 > t2)
			std::swap(t1, t2);
		tMin = std::max(tMin, t1);
		tMax = std::min(tMax, t2);
		if (tMin > tMax)
			return false;
	}

	// Z軸のチェック
	if (std::abs(dir.z) < 0.00001f) {
		if (start.z < aabb.min.z || start.z > aabb.max.z)
			return false;
	} else {
		float t1 = (aabb.min.z - start.z) / dir.z;
		float t2 = (aabb.max.z - start.z) / dir.z;
		if (t1 > t2)
			std::swap(t1, t2);
		tMin = std::max(tMin, t1);
		tMax = std::min(tMax, t2);
		if (tMin > tMax)
			return false;
	}

	return true; // 交差している
}