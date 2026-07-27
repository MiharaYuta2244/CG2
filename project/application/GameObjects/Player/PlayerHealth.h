#pragma once
#include "GameTimer.h"

/// <summary>
/// プレイヤーのHP管理クラス
/// </summary>
class PlayerHealth {
public:
	// 初期化処理
	void Initialize(float maxHP);

	// 更新処理
	void Update(float deltaTime);

	// ダメージ処理
	void Damage(float value);

	// 回復処理
	void Heal(float value);

	// 回復処理
	void AllHeal();

	// 死亡フラグGetter
	bool IsDead() const;

	// 現在のHP取得Getter
	float GetCurrentHP() const { return hp_; }

	// HPの最大値取得Getter
	float GetMaxHP() const { return maxHP_; }

	// 無敵状態かどうかGetter
	bool GetIsInvincible() const { return isInvincible_; }

private:
	float maxHP_ = 3.0f; // 最大HP
	float hp_ = maxHP_;  // 現在のHP
	float preHP_ = hp_;  // 前フレームのHP

	// 無敵状態フラグ
	bool isInvincible_ = false;

	// 無敵時間
	GameTimer invincibleTimer_;

	// 無敵時間
	float invincibleDuration_ = 1.0f;
};
