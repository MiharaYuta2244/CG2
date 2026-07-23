#pragma once
#include "EnemyBomb.h"
#include <memory>
#include <vector>

/// <summary>
/// 敵の爆弾の管理クラス
/// </summary>
class EnemyBombManager {
public:
	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// 爆弾の追加
	void AddBomb(std::unique_ptr<EnemyBomb> bomb);

	// 爆弾のGetter
	const std::vector<std::unique_ptr<EnemyBomb>>& GetBombs() const { return bombs_; }

private:
	std::vector<std::unique_ptr<EnemyBomb>> bombs_;
};