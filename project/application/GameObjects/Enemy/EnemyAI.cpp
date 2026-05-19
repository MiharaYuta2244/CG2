#include "EnemyAI.h"
#include "EnemyBulletManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Wall/WallManager.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>

namespace {
struct PathNode {
	int gridX;
	int gridZ;
	float gCost = 0.0f; // スタートからの実コスト
	float hCost = 0.0f; // ゴールまでの推定コスト(ヒューリスティック)
	PathNode* parent = nullptr;

	float FCost() const { return gCost + hCost; }

	// priority_queue用（Fコストが小さいものを優先）
	bool operator>(const PathNode& other) const { return FCost() > other.FCost(); }
};

// グリッド座標をハッシュ化するための関数
struct GridHash {
	std::size_t operator()(const std::pair<int, int>& k) const { return std::hash<int>()(k.first) ^ (std::hash<int>()(k.second) << 1); }
};
} // namespace

void EnemyAI::Initialize(Transform* transform, EngineContext* ctx) {
	transform_ = transform;
	ctx_ = ctx;

	// 弾の発射間隔を設定
	shotTimer_.Initialize(shotDuration_);

	// 警戒時間を設定
	vigilanceTimer_.Initialize(vigilanceDuration_);
}

void EnemyAI::Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager) {
	Vector3 playerPos = player->GetPosition();
	Vector3 enemyPos = transform_->translate;

	// プレイヤーとの平面距離を計算
	Vector3 toPlayer = {playerPos.x - enemyPos.x, 0.0f, playerPos.z - enemyPos.z};
	float distToPlayer = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);

	// 射線の確保状況
	bool hasLineOfSight = CheckLineOfSight(enemyPos, playerPos, wallManager);

	switch (state_) {
	case State::Normal: {
		// 索敵処理 (前方AABB内にプレイヤーが入ったか)
		AABB sArea = GetSearchArea();
		AABB pCol = player->GetBodyCol();
		bool isHit = (sArea.max.x >= pCol.min.x && sArea.min.x <= pCol.max.x && sArea.max.y >= pCol.min.y && sArea.min.y <= pCol.max.y && sArea.max.z >= pCol.min.z && sArea.min.z <= pCol.max.z);

		if (isHit) {
			state_ = State::Vigilance;
			vigilanceTimer_.Initialize(vigilanceDuration_); // 警戒タイマーセット
			color_ = {1.0f, 0.0f, 0.0f, 1.0f};
		}
		break;
	}

	case State::Vigilance: {
		vigilanceTimer_.Update(deltaTime);

		// 射程圏内で、かつ壁に遮られていなければ射撃モードへ
		if (distToPlayer <= attackRange_ && hasLineOfSight) {
			state_ = State::Shot;
			shotTimer_.Initialize(shotDuration_); // 即撃ちを防ぐためタイマーリセット
			currentPath_.clear();                 // 射撃モード移行時は経路リセット
			color_ = {1.0f, 1.0f, 0.0f, 1.0f};
		} else {
			// A*アルゴリズムを使用したプレイヤーの追跡
			if (distToPlayer > 0.01f) {
				// 一定間隔で経路を計算
				pathRecalculateTimer_ -= deltaTime;
				if (pathRecalculateTimer_ <= 0.0f) {
					currentPath_ = FindPath(enemyPos, playerPos, wallManager);
					currentPathIndex_ = 0;
					pathRecalculateTimer_ = pathRecalculateInterval_;
				}

				// 経路に沿って移動
				if (!currentPath_.empty() && currentPathIndex_ < currentPath_.size()) {
					Vector3 targetPos = currentPath_[currentPathIndex_];
					targetPos.y = enemyPos.y;

					Vector3 toTarget = {targetPos.x - enemyPos.x, 0.0f, targetPos.z - enemyPos.z};
					float distToTarget = std::sqrtf(toTarget.x * toTarget.x + toTarget.z * toTarget.z);

					// ウェイポイントに十分に近づいたら次のポイントへ
					if (distToTarget < 0.5f) {
						currentPathIndex_++;
					} else {
						// 次のウェイポイントに向かって移動
						Vector3 moveDir = MathUtility::Normalize(toTarget);
						transform_->translate.x += moveDir.x * moveSpeed_ * deltaTime;
						transform_->translate.z += moveDir.z * moveSpeed_ * deltaTime;
					}
				}
			}
		}

		// 一定時間経過で通常状態に戻る
		if (vigilanceTimer_.IsEnd()) {
			state_ = State::Normal;
			currentPath_.clear();
			color_ = {1.0f, 1.0f, 1.0f, 1.0f};
		}
		break;
	}

	case State::Shot: {
		// 壁に隠れられたり、射程外に逃げられたら警戒モードに戻る
		if (!hasLineOfSight || distToPlayer > attackRange_) {
			state_ = State::Vigilance;
			vigilanceTimer_.Initialize(vigilanceDuration_);
			break;
		}

		// 既存の射撃処理
		shotTimer_.Update(deltaTime);
		if (shotTimer_.IsEnd()) {
			auto bullet = std::make_unique<EnemyBullet>();
			Vector3 dir = MathUtility::Normalize(toPlayer);
			bullet->Initialize(ctx_, {dir.x, dir.z}, transform_->translate);
			enemyBulletManager->AddBullet(std::move(bullet));

			shotTimer_.Initialize(shotDuration_);
		}
		break;
	}
	}
}

AABB EnemyAI::GetSearchArea() const {
	AABB area;
	Vector3 pos = transform_->translate;
	area.min = {pos.x - searchWidth_ * 0.5f, pos.y - 1.0f, pos.z};
	area.max = {pos.x + searchWidth_ * 0.5f, pos.y + 1.0f, pos.z + searchDistance_};
	return area;
}

bool EnemyAI::CheckLineOfSight(const Vector3& start, const Vector3& end, WallManager* wallManager) {
	Vector3 dir = {end.x - start.x, end.y - start.y, end.z - start.z};

	for (const auto& wall : wallManager->GetWalls()) {
		AABB aabb = wall->GetCollision();
		float tmin = 0.0f;
		float tmax = 1.0f;

		auto checkAxis = [&](float pStart, float pDir, float bMin, float bMax) -> bool {
			if (std::abs(pDir) < 0.0001f) {
				return (pStart >= bMin && pStart <= bMax);
			}
			float t1 = (bMin - pStart) / pDir;
			float t2 = (bMax - pStart) / pDir;
			if (t1 > t2)
				std::swap(t1, t2);
			if (tmin < t1)
				tmin = t1;
			if (tmax > t2)
				tmax = t2;
			return tmin <= tmax;
		};

		if (!checkAxis(start.x, dir.x, aabb.min.x, aabb.max.x))
			continue;
		if (!checkAxis(start.y, dir.y, aabb.min.y, aabb.max.y))
			continue;
		if (!checkAxis(start.z, dir.z, aabb.min.z, aabb.max.z))
			continue;

		// 1つでも交差していれば射線が遮られている
		return false;
	}
	return true; // 全ての壁をすり抜けた＝射線が通っている
}

std::vector<Vector3> EnemyAI::FindPath(const Vector3& startPos, const Vector3& targetPos, WallManager* wallManager) {
	float gridSize = 1.0f;

	// ワールド座標をグリッドのインデックスに変換
	auto ToGrid = [gridSize](float val) { return static_cast<int>(std::round(val / gridSize)); };
	auto ToWorld = [gridSize](int val) { return static_cast<float>(val) * gridSize; };

	int startX = ToGrid(startPos.x);
	int startZ = ToGrid(startPos.z);
	int targetX = ToGrid(targetPos.x);
	int targetZ = ToGrid(targetPos.z);

	// オープンリストとクローズドリスト
	std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> openList;
	std::unordered_map<std::pair<int, int>, PathNode*, GridHash> allNodes; // メモリ管理用
	std::unordered_map<std::pair<int, int>, bool, GridHash> closedList;

	auto CreateNode = [&](int x, int z) -> PathNode* {
		auto key = std::make_pair(x, z);
		if (allNodes.find(key) == allNodes.end()) {
			PathNode* node = new PathNode{x, z};
			allNodes[key] = node;
		}
		return allNodes[key];
	};

	PathNode* startNode = CreateNode(startX, startZ);
	openList.push(*startNode);

	PathNode* goalNode = nullptr;

	// 探索方向
	int dx[] = {0, 1, 0, -1, 1, 1, -1, -1};
	int dz[] = {1, 0, -1, 0, 1, -1, 1, -1};

	int maxIterations = 500; // 処理落ち対策　探索回数上限
	int iterations = 0;

	while (!openList.empty() && iterations < maxIterations) {
		PathNode currentCopy = openList.top();
		openList.pop();
		PathNode* current = allNodes[std::make_pair(currentCopy.gridX, currentCopy.gridZ)];

		auto currentKey = std::make_pair(current->gridX, current->gridZ);
		if (closedList[currentKey])
			continue; // 探索済みならスキップ
		closedList[currentKey] = true;
		iterations++;

		// ゴール到達判定
		if (current->gridX == targetX && current->gridZ == targetZ) {
			goalNode = current;
			break;
		}

		// 周囲のノード展開
		for (int i = 0; i < 8; i++) {
			int nx = current->gridX + dx[i];
			int nz = current->gridZ + dz[i];
			auto neighborKey = std::make_pair(nx, nz);

			if (closedList[neighborKey])
				continue;

			// ワールド座標に戻して障害物判定
			Vector3 worldPos = {ToWorld(nx), startPos.y, ToWorld(nz)};
			if (!IsWalkable(worldPos, wallManager))
				continue;

			// 斜め移動はコストを高くする
			float moveCost = (dx[i] != 0 && dz[i] != 0) ? 1.414f : 1.0f;
			float newGCost = current->gCost + moveCost;

			PathNode* neighbor = CreateNode(nx, nz);

			if (newGCost < neighbor->gCost || neighbor->gCost == 0.0f) {
				neighbor->gCost = newGCost;

				// ヒューリスティックコスト計算
				neighbor->hCost = static_cast<float>(std::abs(nx - targetX) + std::abs(nz - targetZ));
				neighbor->parent = current;
				openList.push(*neighbor);
			}
		}
	}

	// 経路の復元 (ゴールから親を辿ってスタートへ)
	std::vector<Vector3> path;
	if (goalNode != nullptr) {
		PathNode* curr = goalNode;
		while (curr != nullptr && curr != startNode) {
			path.push_back({ToWorld(curr->gridX), startPos.y, ToWorld(curr->gridZ)});
			curr = curr->parent;
		}
		std::reverse(path.begin(), path.end()); // スタートからゴールの順に反転
	}

	// メモリ開放
	for (auto& pair : allNodes) {
		delete pair.second;
	}

	return path;
}

bool EnemyAI::IsWalkable(const Vector3& pos, WallManager* wallManager) const {
	// 敵の半径
	float enemyRadius = 0.5f;

	// 仮の敵のAABB
	AABB enemyCol;
	enemyCol.min = {pos.x - enemyRadius, pos.y, pos.z - enemyRadius};
	enemyCol.max = {pos.x + enemyRadius, pos.y + 2.0f, pos.z + enemyRadius};

	// すべての壁との交差判定
	for (const auto& wall : wallManager->GetWalls()) {
		AABB wCol = wall->GetCollision();
		bool isHit =
		    (enemyCol.max.x > wCol.min.x && enemyCol.min.x < wCol.max.x && enemyCol.max.y > wCol.min.y && enemyCol.min.y < wCol.max.y && enemyCol.max.z > wCol.min.z && enemyCol.min.z < wCol.max.z);
		if (isHit) {
			return false; // 壁と衝突しているため通行不可
		}
	}
	return true;
}
