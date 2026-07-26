#pragma once
#include "Vector3.h"
#include <vector>
#include "GameObjects/GameObjectManager.h"
#include "GameObjects/StageObjects/Wall/Wall.h"

using WallManager = GameObjectManager<Wall>;

// 1つのマス目を表す構造体
struct PathNode {
	int gridX;             // グリッド上のXインデックス
	int gridZ;             // グリッド上のZインデックス
	Vector3 worldPosition; // このマスのワールド座標の中心
	bool isWalkable;       // 歩けるマスかどうか（壁がないか）

	// A*計算用コスト
	float gCost;      // スタートからの実際の移動距離コスト
	float hCost;      // ゴールまでの推定距離コスト（ヒューリスティック）
	PathNode* parent; // 経路を遡るための親ノードポインタ

	// fCostは gCost + hCost
	float fCost() const { return gCost + hCost; }

	// コンストラクタ
	PathNode(bool _isWalkable, Vector3 _worldPos, int _gridX, int _gridZ) : isWalkable(_isWalkable), worldPosition(_worldPos), gridX(_gridX), gridZ(_gridZ), gCost(0), hCost(0), parent(nullptr) {}
};

class AStarPathfinder {
public:
	// スタート座標からゴール座標までの経路リストを計算して返す
	static std::vector<Vector3> FindPath(const Vector3& startPos, const Vector3& targetPos, WallManager* wallManager);

private:
	// ワールド座標からグリッドのインデックスに変換
	static PathNode* GetNodeFromWorldPoint(const Vector3& worldPosition, std::vector<std::vector<PathNode>>& grid, const Vector3& gridBottomLeft, int gridSizeX, int gridSizeZ, float nodeDiameter);

	// 隣接するマスを取得
	static std::vector<PathNode*> GetNeighbors(PathNode* node, std::vector<std::vector<PathNode>>& grid, int gridSizeX, int gridSizeZ);

	// ノード間の距離を計算
	static float GetDistance(PathNode* nodeA, PathNode* nodeB);

	// そのマスが壁と重なっていないかを判定
	static bool CheckWalkable(const Vector3& nodePos, float nodeRadius, WallManager* wallManager);
};