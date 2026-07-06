#pragma once
#include "EnemyBullet.h"
#include "Rect.h"
#include <list>
#include <memory>

/// <summary>
/// 敵の弾を管理するクラス
/// </summary>
class EnemyBulletManager {
public:
	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// 弾を追加する関数
	void AddBullet(std::unique_ptr<EnemyBullet> bullet);

	// 当たり判定用に弾のリストを取得
	const std::list<std::unique_ptr<EnemyBullet>>& GetBullets() const { return bullets_; }

private:
	// 弾のリスト
	std::list<std::unique_ptr<EnemyBullet>> bullets_;

	// 弾が生存できる範囲
	Rect<float> bulletActiveArea_ = {100.0f, -100.0f, 100.0f, -100.0f};
};