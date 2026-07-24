#pragma once
#include "Vector3.h"
#include <functional>

class Player;
class EnemyManager;
class EnemyBulletManager;
class EnemyBombManager;
class Stage;
struct CommonData;
class Camera;

/// <summary>
/// ゲームプレイ中の全当たり判定を管理するクラス
/// </summary>
class CollisionManager {
public:
	/// <summary>
	/// すべての当たり判定をチェックして応答処理を行う
	/// </summary>
	void CheckCollisions(
	    Player* player, EnemyManager* enemyManager, EnemyBulletManager* enemyBulletManager, EnemyBombManager * enemyBombManager, Stage * stage, Camera* camera, CommonData* commonData,
	    std::function<void(const Vector3&)> generateParticleCallback, float& glitchTimer);
};