#include "EnemyBulletManager.h"

void EnemyBulletManager::Update(float deltaTime) {
	for (auto& bullet : bullets_) {
		bullet->Update(deltaTime);
	}

	// 弾の削除
	std::erase_if(bullets_, [this](const std::unique_ptr<EnemyBullet>& b) { return b->IsDead(bulletActiveArea_); });
}

void EnemyBulletManager::Draw() {
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
}

void EnemyBulletManager::AddBullet(std::unique_ptr<EnemyBullet> bullet) { bullets_.push_back(std::move(bullet)); }
