#pragma once
#include "Enemy.h"
#include <list>
#include <memory>
#include <string>

class Player;
class EnemyBulletManager;
class WallManager;
class DoorManager;

struct EnemyData {
	Vector3 pos;
	Vector3 rot;
};

inline void to_json(Json& j, const EnemyData& e) {
	j = Json{
	    {"pos", e.pos},
        {"rot", e.rot}
    };
}

inline void from_json(const Json& j, EnemyData& e) {
	e.pos = j.at("pos").get<Vector3>();
	e.rot = j.at("rot").get<Vector3>();
}

/// <summary>
/// 敵管理クラス
/// </summary>
class EnemyManager {
public:
	void Initialize(EngineContext* ctx);
	void Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager, DoorManager* doorManager);
	void PostUpdate();
	void Draw();
	void DrawImGui();
	void SetMove();
	void SetStop();

	// 敵のリストを取得するGetter
	std::list<std::unique_ptr<Enemy>>& GetEnemies() { return enemies_; }

private:
	void LoadFromJson(const std::string& filepath);
	void SaveToJson(const std::string& filepath);

private:
	EngineContext* ctx_ = nullptr;
	std::string jsonPath_;

	// 敵オブジェクトのリスト
	std::list<std::unique_ptr<Enemy>> enemies_;
};