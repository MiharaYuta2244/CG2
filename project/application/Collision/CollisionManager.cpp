#include "CollisionManager.h"
#include "Collision.h"
#include "DebugCamera.h"
#include "GameObjects/CommonData.h"
#include "GameObjects/Enemy/EnemyBombManager.h"
#include "GameObjects/Enemy/EnemyBulletManager.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/StageObjects/Stage.h"
#include <algorithm>

using namespace TinyEngine;

void CollisionManager::CheckCollisions(
    Player* player, EnemyManager* enemyManager, EnemyBulletManager* enemyBulletManager, EnemyBombManager* enemyBombManager, Stage* stage, Camera* camera, CommonData* commonData,
    std::function<void(const Vector3&)> generateParticleCallback, float& glitchTimer) {

	// ==========================================
	// プレイヤーと敵の弾の当たり判定
	// ==========================================
	for (auto& bullet : enemyBulletManager->GetBullets()) {
		if (!bullet->IsCollisionActive())
			continue;

		if (Collision::Intersect(player->GetBodyCol(), bullet->GetCollision())) {
			bool isGuarded = false;

			if (!camera->GetIsShake()) {
				camera->StartShake(0.1f, 0.1f);
			}

			if (player->IsGrabbingEnemy()) {
				Vector2 playerDir = player->GetDirection();
				Vector3 playerToBullet = MathUtility::Normalize(bullet->GetPosition() - player->GetPosition());
				float dot = playerDir.x * playerToBullet.x + playerDir.y * playerToBullet.z;

				if (dot < 0.0f) {
					isGuarded = true;
					Enemy* shieldEnemy = player->GetGrabbedEnemy();
					if (shieldEnemy) {
						shieldEnemy->Damage();
						if (shieldEnemy->IsDead()) {
							commonData->killCount += 1;
							generateParticleCallback(shieldEnemy->GetPos());
						}
						player->SetIsGrabReleased(true);
						player->SetIsHold(false);
					}
				}
			}

			if (!isGuarded) {
				player->Damage(1);
				glitchTimer = 0.5f;
			}
		}
	}

	// ==========================================
	// プレイヤーの攻撃用範囲と敵の当たり判定
	// ==========================================
	for (auto& enemy : enemyManager->GetEnemies()) {
		if (Collision::Intersect(player->GetAttackCol(), enemy->GetBodyCol())) {
			// プレイヤーの攻撃フラグを立てる
			player->SetEnableAttack(true);
		}
	}

	// ==========================================
	// プレイヤーの攻撃用範囲とガラスの当たり判定
	// ==========================================
	if (player->GetIsAttack()) {
		for (auto& glass : stage->GetGlassManager()->GetGlasses()) {
			if (Collision::Intersect(player->GetAttackCol(), glass->GetCollision())) {
				// ガラス削除
				glass->AddGlassesDecal({1, 1, 1});
				stage->GetGlassManager()->RemoveGlass(glass.get());
				break;
			}
		}
	}

	// ==========================================
	// プレイヤーと壁の押し出し判定
	// ==========================================
	AABB playerAABB = player->GetBodyCol();
	Vector3 playerPos = player->GetPosition();

	for (const auto& wall : stage->GetWallManager()->GetObjects()) {
		AABB wallAABB = wall->GetCollision();

		// AABB同士の交差判定
		if (playerAABB.min.x <= wallAABB.max.x && playerAABB.max.x >= wallAABB.min.x && playerAABB.min.y <= wallAABB.max.y && playerAABB.max.y >= wallAABB.min.y &&
		    playerAABB.min.z <= wallAABB.max.z && playerAABB.max.z >= wallAABB.min.z) {

			// めり込み量の計算
			// X軸方向のめり込み量
			float overlapX1 = wallAABB.max.x - playerAABB.min.x; // 右から左へ押す量
			float overlapX2 = playerAABB.max.x - wallAABB.min.x; // 左から右へ押す量
			// Z軸方向のめり込み量
			float overlapZ1 = wallAABB.max.z - playerAABB.min.z; // 奥から手前へ押す量
			float overlapZ2 = playerAABB.max.z - wallAABB.min.z; // 手前から奥へ押す量

			// 最小のめり込み量を選ぶ（正の値にする）
			float minOverlapX = std::min(overlapX1, overlapX2);
			float minOverlapZ = std::min(overlapZ1, overlapZ2);

			// めり込みが少ない軸の方向に押し出す
			if (minOverlapX < minOverlapZ) {
				// X軸方向に押し出す
				if (overlapX1 < overlapX2) {
					playerPos.x += overlapX1; // 右へ押し出す
				} else {
					playerPos.x -= overlapX2; // 左へ押し出す
				}
			} else {
				// Z軸方向に押し出す
				if (overlapZ1 < overlapZ2) {
					playerPos.z += overlapZ1; // 奥へ押し出す
				} else {
					playerPos.z -= overlapZ2; // 手前へ押し出す
				}
			}

			// 押し出した結果をプレイヤーの座標に反映
			player->SetPosition(playerPos);

			// 複数の壁と連続で当たるケースを考慮し、判定用AABBもその場で更新
			playerAABB.max.x = playerPos.x + 0.5f;
			playerAABB.min.x = playerPos.x - 0.5f;
			playerAABB.max.z = playerPos.z + 0.5f;
			playerAABB.min.z = playerPos.z - 0.5f;
		}
	}

	// ==========================================
	// プレイヤーとガラスの押し出し判定
	// ==========================================
	for (const auto& glass : stage->GetGlassManager()->GetGlasses()) {
		AABB glassAABB = glass->GetCollision();

		// AABB同士の交差判定
		if (playerAABB.min.x <= glassAABB.max.x && playerAABB.max.x >= glassAABB.min.x && playerAABB.min.y <= glassAABB.max.y && playerAABB.max.y >= glassAABB.min.y &&
		    playerAABB.min.z <= glassAABB.max.z && playerAABB.max.z >= glassAABB.min.z) {

			// めり込み量の計算
			// X軸方向のめり込み量
			float overlapX1 = glassAABB.max.x - playerAABB.min.x; // 右から左へ押す量
			float overlapX2 = playerAABB.max.x - glassAABB.min.x; // 左から右へ押す量
			// Z軸方向のめり込み量
			float overlapZ1 = glassAABB.max.z - playerAABB.min.z; // 奥から手前へ押す量
			float overlapZ2 = playerAABB.max.z - glassAABB.min.z; // 手前から奥へ押す量

			// 最小のめり込み量を選ぶ（正の値にする）
			float minOverlapX = std::min(overlapX1, overlapX2);
			float minOverlapZ = std::min(overlapZ1, overlapZ2);

			// めり込みが少ない軸の方向に押し出す
			if (minOverlapX < minOverlapZ) {
				// X軸方向に押し出す
				if (overlapX1 < overlapX2) {
					playerPos.x += overlapX1; // 右へ押し出す
				} else {
					playerPos.x -= overlapX2; // 左へ押し出す
				}
			} else {
				// Z軸方向に押し出す
				if (overlapZ1 < overlapZ2) {
					playerPos.z += overlapZ1; // 奥へ押し出す
				} else {
					playerPos.z -= overlapZ2; // 手前へ押し出す
				}
			}

			// 押し出した結果をプレイヤーの座標に反映
			player->SetPosition(playerPos);

			// 複数の壁と連続で当たるケースを考慮し、判定用AABBもその場で更新
			playerAABB.max.x = playerPos.x + 0.5f;
			playerAABB.min.x = playerPos.x - 0.5f;
			playerAABB.max.z = playerPos.z + 0.5f;
			playerAABB.min.z = playerPos.z - 0.5f;
		}
	}

	// ==========================================
	// プレイヤーと檻の押し出し判定
	// ==========================================
	for (const auto& cage : stage->GetCageManager()->GetCages()) {
		AABB cageAABB = cage->GetCollision();

		// AABB同士の交差判定
		if (playerAABB.min.x <= cageAABB.max.x && playerAABB.max.x >= cageAABB.min.x && playerAABB.min.y <= cageAABB.max.y && playerAABB.max.y >= cageAABB.min.y &&
		    playerAABB.min.z <= cageAABB.max.z && playerAABB.max.z >= cageAABB.min.z) {

			// めり込み量の計算
			// X軸方向のめり込み量
			float overlapX1 = cageAABB.max.x - playerAABB.min.x; // 右から左へ押す量
			float overlapX2 = playerAABB.max.x - cageAABB.min.x; // 左から右へ押す量
			// Z軸方向のめり込み量
			float overlapZ1 = cageAABB.max.z - playerAABB.min.z; // 奥から手前へ押す量
			float overlapZ2 = playerAABB.max.z - cageAABB.min.z; // 手前から奥へ押す量

			// 最小のめり込み量を選ぶ（正の値にする）
			float minOverlapX = std::min(overlapX1, overlapX2);
			float minOverlapZ = std::min(overlapZ1, overlapZ2);

			// めり込みが少ない軸の方向に押し出す
			if (minOverlapX < minOverlapZ) {
				// X軸方向に押し出す
				if (overlapX1 < overlapX2) {
					playerPos.x += overlapX1; // 右へ押し出す
				} else {
					playerPos.x -= overlapX2; // 左へ押し出す
				}
			} else {
				// Z軸方向に押し出す
				if (overlapZ1 < overlapZ2) {
					playerPos.z += overlapZ1; // 奥へ押し出す
				} else {
					playerPos.z -= overlapZ2; // 手前へ押し出す
				}
			}

			// 押し出した結果をプレイヤーの座標に反映
			player->SetPosition(playerPos);

			// 複数の壁と連続で当たるケースを考慮し、判定用AABBもその場で更新
			playerAABB.max.x = playerPos.x + 0.5f;
			playerAABB.min.x = playerPos.x - 0.5f;
			playerAABB.max.z = playerPos.z + 0.5f;
			playerAABB.min.z = playerPos.z - 0.5f;
		}
	}

	// ==========================================
	// 敵と壁の判定（めり込み防止 ＆ ノックバック時死亡）
	// ==========================================
	for (auto& enemy : enemyManager->GetEnemies()) {
		if (enemy->IsDead())
			continue;

		AABB enemyAABB = enemy->GetBodyCol();
		Vector3 enemyPos = enemy->GetPos();

		for (const auto& wall : stage->GetWallManager()->GetObjects()) {
			AABB wallAABB = wall->GetCollision();

			// AABB同士の交差判定
			if (enemyAABB.min.x <= wallAABB.max.x && enemyAABB.max.x >= wallAABB.min.x && enemyAABB.min.y <= wallAABB.max.y && enemyAABB.max.y >= wallAABB.min.y && enemyAABB.min.z <= wallAABB.max.z &&
			    enemyAABB.max.z >= wallAABB.min.z) {

				// ----------------------------------------
				// 壁激突死の判定：もしノックバック中なら死亡させて処理を抜ける
				// ----------------------------------------
				if (enemy->IsKnockBack()) {
					enemy->Kill();
					commonData->killCount++;

					// パーティクルの生成
					generateParticleCallback(enemy->GetPos());

					break;
				}

				// ----------------------------------------
				// 通常時の押し出し処理
				// ----------------------------------------
				float overlapX1 = wallAABB.max.x - enemyAABB.min.x;
				float overlapX2 = enemyAABB.max.x - wallAABB.min.x;
				float overlapZ1 = wallAABB.max.z - enemyAABB.min.z;
				float overlapZ2 = enemyAABB.max.z - wallAABB.min.z;

				float minOverlapX = std::min(overlapX1, overlapX2);
				float minOverlapZ = std::min(overlapZ1, overlapZ2);

				if (minOverlapX < minOverlapZ) {
					if (overlapX1 < overlapX2) {
						enemyPos.x += overlapX1;
					} else {
						enemyPos.x -= overlapX2;
					}
				} else {
					if (overlapZ1 < overlapZ2) {
						enemyPos.z += overlapZ1;
					} else {
						enemyPos.z -= overlapZ2;
					}
				}

				// 結果を敵の座標に反映
				enemy->SetPos(enemyPos);

				// 連続で壁に当たるケースを考慮してAABB更新
				enemyAABB.max.x = enemyPos.x + 0.5f;
				enemyAABB.min.x = enemyPos.x - 0.5f;
				enemyAABB.max.z = enemyPos.z + 0.5f;
				enemyAABB.min.z = enemyPos.z - 0.5f;
			}
		}
	}

	// =========================================================
	// 敵とドアの当たり判定
	// =========================================================
	for (auto& enemy : enemyManager->GetEnemies()) {
		for (auto& door : stage->GetDoorManager()->GetObjects()) {
			AABB wallAABB = door->GetCollision();
			AABB enemyAABB = enemy->GetBodyCol();
			Vector3 enemyPos = enemy->GetPos();

			// ドアが開いているなら敵は通り抜けられるのでスキップ
			if (door->GetIsOpen()) {
				continue;
			}

			// 衝突判定
			if (Collision::Intersect(enemy->GetBodyCol(), door->GetCollision())) {
				// 押し戻し・衝突応答処理
				// ----------------------------------------
				// 壁激突死の判定：もしノックバック中なら死亡させて処理を抜ける
				// ----------------------------------------
				if (enemy->IsKnockBack()) {
					enemy->Kill();
					commonData->killCount++;

					// パーティクルの生成
					generateParticleCallback(enemy->GetPos());

					break;
				}

				// ----------------------------------------
				// 通常時の押し出し処理
				// ----------------------------------------
				float overlapX1 = wallAABB.max.x - enemyAABB.min.x;
				float overlapX2 = enemyAABB.max.x - wallAABB.min.x;
				float overlapZ1 = wallAABB.max.z - enemyAABB.min.z;
				float overlapZ2 = enemyAABB.max.z - wallAABB.min.z;

				float minOverlapX = std::min(overlapX1, overlapX2);
				float minOverlapZ = std::min(overlapZ1, overlapZ2);

				if (minOverlapX < minOverlapZ) {
					if (overlapX1 < overlapX2) {
						enemyPos.x += overlapX1;
					} else {
						enemyPos.x -= overlapX2;
					}
				} else {
					if (overlapZ1 < overlapZ2) {
						enemyPos.z += overlapZ1;
					} else {
						enemyPos.z -= overlapZ2;
					}
				}

				// 結果を敵の座標に反映
				enemy->SetPos(enemyPos);

				// 連続で壁に当たるケースを考慮してAABB更新
				enemyAABB.max.x = enemyPos.x + 0.5f;
				enemyAABB.min.x = enemyPos.x - 0.5f;
				enemyAABB.max.z = enemyPos.z + 0.5f;
				enemyAABB.min.z = enemyPos.z - 0.5f;
			}
		}
	}

	// ==========================================
	// プレイヤーとゴールの当たり判定
	// ==========================================
	if (Collision::Intersect(player->GetBodyCol(), stage->GetGoal()->GetCollision())) {
		// ゴールフラグを立てる
		stage->GetGoal()->SetIsGoal(true);
	}

	// ==========================================
	// 敵同士の当たり判定
	// ==========================================
	auto& enemies = enemyManager->GetEnemies();

	for (auto itA = enemies.begin(); itA != enemies.end(); ++itA) {
		Enemy* a = itA->get();
		if (a->IsDead())
			continue;

		auto itB = itA;
		++itB;

		for (; itB != enemies.end(); ++itB) {
			Enemy* b = itB->get();
			if (b->IsDead())
				continue;

			// 当たり判定
			if (Collision::Intersect(a->GetBodyCol(), b->GetBodyCol())) {
				// どちらかがノックバック状態であれば
				if (a->IsKnockBack() || b->IsKnockBack()) {
					a->Damage();
					b->Damage();
					a->StopAnimation();
					b->StopAnimation();

					// ダメージの結果死亡した場合のみ処理を行う
					if (a->IsDead()) {
						commonData->killCount += 1;
						generateParticleCallback(a->GetPos());
					}
					if (b->IsDead()) {
						commonData->killCount += 1;
						generateParticleCallback(b->GetPos());
					}
				}
			}
		}
	}

	// ==========================================
	// 敵と敵の弾の当たり判定
	// ==========================================
	if (player->GetGrabbedEnemy() != nullptr) {
		for (const auto& enemy : enemyManager->GetEnemies()) {
			for (const auto& bullet : enemyBulletManager->GetBullets()) {
				// 発射された1フレーム目以外は判定を行わない
				if (!bullet->IsCollisionActive()) {
					continue;
				}

				// 衝突判定
				if (Collision::Intersect(enemy->GetBodyCol(), bullet->GetCollision())) {
					enemy->Damage();

					if (enemy->IsDead()) {
						commonData->killCount += 1;
						generateParticleCallback(enemy->GetPos());
					}
				}
			}
		}
	}

	// ==========================================
	// プレイヤーと回復エリアの当たり判定
	// ==========================================
	for (const auto& healArea : stage->GetHealAreaManager()->GetHealAreas()) {
		if (Collision::Intersect(player->GetBodyCol(), healArea->GetCollision())) {
			player->AllHeal();
			healArea->SetIsActive(false);
		}
	}

	// ==========================================
	// プレイヤーと敵の爆弾の当たり判定
	// ==========================================
	for (auto& bomb : enemyBombManager->GetBombs()) {
		if (Collision::Intersect(player->GetBodyCol(), bomb->GetBombCollision())) {
			if (!camera->GetIsShake()) {
				camera->StartShake(0.1f, 0.1f);
			}

			if (bomb->IsExploded()) {
				// プレイヤー死亡
				player->Damage(3);
			}
		}
	}
}