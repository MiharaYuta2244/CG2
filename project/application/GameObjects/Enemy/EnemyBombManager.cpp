#include "EnemyBombManager.h"

void EnemyBombManager::Update(float deltaTime) {
	for (auto& bomb : bombs_) {
		bomb->Update(deltaTime);
	}
	std::erase_if(bombs_, [](const std::unique_ptr<EnemyBomb>& b) { return b->IsFinished(); });
}

void EnemyBombManager::Draw() {
	for (auto& bomb : bombs_) {
		bomb->Draw();
	}
}

void EnemyBombManager::AddBomb(std::unique_ptr<EnemyBomb> bomb) { bombs_.push_back(std::move(bomb)); }