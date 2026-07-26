#include "AStarPathfinder.h"
#include <algorithm>
#include <cmath>

// グリッド生成用の定数設定
const float NODE_RADIUS = 0.5f;                 // 1マスの半径
const float NODE_DIAMETER = NODE_RADIUS * 2.0f; // 1マスのサイズ
const int GRID_SIZE_X = 40;                     // 探索範囲のX方向のマス数
const int GRID_SIZE_Z = 40;                     // 探索範囲のZ方向のマス数

std::vector<Vector3> AStarPathfinder::FindPath(const Vector3& startPos, const Vector3& targetPos, WallManager* wallManager) {
	// 探索用グリッドの動的生成
	Vector3 gridBottomLeft = startPos;
	gridBottomLeft.x -= (GRID_SIZE_X * NODE_RADIUS);
	gridBottomLeft.z -= (GRID_SIZE_Z * NODE_RADIUS);
	gridBottomLeft.y = 0.0f;

	// 2次元配列でグリッドを確保
	std::vector<std::vector<PathNode>> grid;
	grid.resize(GRID_SIZE_X);
	for (int x = 0; x < GRID_SIZE_X; ++x) {
		grid[x].reserve(GRID_SIZE_Z);
		for (int z = 0; z < GRID_SIZE_Z; ++z) {
			Vector3 worldPoint = gridBottomLeft;
			worldPoint.x += (x * NODE_DIAMETER + NODE_RADIUS);
			worldPoint.z += (z * NODE_DIAMETER + NODE_RADIUS);

			// マスが壁と重なっているかチェック
			bool walkable = CheckWalkable(worldPoint, NODE_RADIUS, wallManager);
			grid[x].emplace_back(walkable, worldPoint, x, z);
		}
	}

	// スタートとターゲットのノードを取得
	PathNode* startNode = GetNodeFromWorldPoint(startPos, grid, gridBottomLeft, GRID_SIZE_X, GRID_SIZE_Z, NODE_DIAMETER);
	PathNode* targetNode = GetNodeFromWorldPoint(targetPos, grid, gridBottomLeft, GRID_SIZE_X, GRID_SIZE_Z, NODE_DIAMETER);

	// 探索範囲外（グリッドの外）の場合は空の経路を返す
	if (!startNode || !targetNode || !startNode->isWalkable)
		return {};

	// A* の探索準備 (OpenリストとClosedリスト)
	std::vector<PathNode*> openSet; // 評価候補のノード
	// Closedリストは検索を高速化するため bool の二次元配列を使用
	std::vector<std::vector<bool>> closedSet(GRID_SIZE_X, std::vector<bool>(GRID_SIZE_Z, false));

	openSet.push_back(startNode);

	// メインループ
	while (!openSet.empty()) {
		// Openリストの中で最もfCostが低いノードを取り出す
		PathNode* currentNode = openSet[0];
		int currentIndex = 0;
		for (int i = 1; i < openSet.size(); ++i) {
			if (openSet[i]->fCost() < currentNode->fCost() || (openSet[i]->fCost() == currentNode->fCost() && openSet[i]->hCost < currentNode->hCost)) {
				currentNode = openSet[i];
				currentIndex = i;
			}
		}

		// 現在のノードをOpenから外し、Closedに入れる
		openSet.erase(openSet.begin() + currentIndex);
		closedSet[currentNode->gridX][currentNode->gridZ] = true;

		// ゴールに到達したか判定
		if (currentNode == targetNode) {
			// 経路をたどってリスト（ウェイポイント）を作成
			std::vector<Vector3> path;
			PathNode* current = targetNode;
			while (current != startNode) {
				path.push_back(current->worldPosition);
				current = current->parent;
			}
			std::reverse(path.begin(), path.end()); // ゴール→スタートになっているので反転する
			return path;
		}

		// 隣接する周囲8方向のマスを調べる
		for (PathNode* neighbor : GetNeighbors(currentNode, grid, GRID_SIZE_X, GRID_SIZE_Z)) {
			// 歩けない（壁）、または既に評価済み(Closed)ならスキップ
			if (!neighbor->isWalkable || closedSet[neighbor->gridX][neighbor->gridZ]) {
				continue;
			}

			// 現在のノード経由で隣のマスへ行く時のコスト
			float newMovementCostToNeighbor = currentNode->gCost + GetDistance(currentNode, neighbor);

			// Openリストに含まれているか
			auto it = std::find(openSet.begin(), openSet.end(), neighbor);
			bool isInOpenSet = (it != openSet.end());

			// より短い経路が見つかった、もしくはまだOpenリストに入っていない場合
			if (newMovementCostToNeighbor < neighbor->gCost || !isInOpenSet) {
				neighbor->gCost = newMovementCostToNeighbor;
				neighbor->hCost = GetDistance(neighbor, targetNode);
				neighbor->parent = currentNode;

				if (!isInOpenSet) {
					openSet.push_back(neighbor);
				}
			}
		}
	}

	// 経路が見つからなかった（完全に壁で囲まれているなど）
	return {};
}

PathNode*
    AStarPathfinder::GetNodeFromWorldPoint(const Vector3& worldPosition, std::vector<std::vector<PathNode>>& grid, const Vector3& gridBottomLeft, int gridSizeX, int gridSizeZ, float nodeDiameter) {
	// グリッドの左下からの割合を計算
	float percentX = (worldPosition.x - gridBottomLeft.x) / (gridSizeX * nodeDiameter);
	float percentZ = (worldPosition.z - gridBottomLeft.z) / (gridSizeZ * nodeDiameter);

	// 0.0 ~ 1.0 の範囲に収める
	percentX = std::clamp(percentX, 0.0f, 1.0f);
	percentZ = std::clamp(percentZ, 0.0f, 1.0f);

	// 配列のインデックスに変換
	int x = static_cast<int>(percentX * (gridSizeX - 1));
	int z = static_cast<int>(percentZ * (gridSizeZ - 1));

	if (x >= 0 && x < gridSizeX && z >= 0 && z < gridSizeZ) {
		return &grid[x][z];
	}
	return nullptr;
}

std::vector<PathNode*> AStarPathfinder::GetNeighbors(PathNode* node, std::vector<std::vector<PathNode>>& grid, int gridSizeX, int gridSizeZ) {
	std::vector<PathNode*> neighbors;

	// 周囲8方向をループ
	for (int x = -1; x <= 1; ++x) {
		for (int z = -1; z <= 1; ++z) {
			if (x == 0 && z == 0)
				continue; // 自分自身は除外

			int checkX = node->gridX + x;
			int checkZ = node->gridZ + z;

			// グリッドの配列範囲内かチェック
			if (checkX >= 0 && checkX < gridSizeX && checkZ >= 0 && checkZ < gridSizeZ) {
				neighbors.push_back(&grid[checkX][checkZ]);
			}
		}
	}

	return neighbors;
}

float AStarPathfinder::GetDistance(PathNode* nodeA, PathNode* nodeB) {
	int dstX = std::abs(nodeA->gridX - nodeB->gridX);
	int dstZ = std::abs(nodeA->gridZ - nodeB->gridZ);

	// A*の定番の距離計算（直線移動コストを10、斜め移動コストを14とする近似計算）
	if (dstX > dstZ)
		return 14.0f * dstZ + 10.0f * (dstX - dstZ);
	return 14.0f * dstX + 10.0f * (dstZ - dstX);
}

bool AStarPathfinder::CheckWalkable(const Vector3& nodePos, float nodeRadius, WallManager* wallManager) {
	const auto& walls = wallManager->GetObjects();

	// マス目の2D AABB（Y軸を無視したXとZの四角形）
	float minX = nodePos.x - nodeRadius;
	float maxX = nodePos.x + nodeRadius;
	float minZ = nodePos.z - nodeRadius;
	float maxZ = nodePos.z + nodeRadius;

	for (const auto& wall : walls) {
		AABB wallCol = wall->GetCollision();
		// 2D平面(XとZ)でのAABB同士の交差判定
		if (minX <= wallCol.max.x && maxX >= wallCol.min.x && minZ <= wallCol.max.z && maxZ >= wallCol.min.z) {
			return false; // 壁と重なっているので歩けない
		}
	}

	return true; // どの壁とも重なっていない
}