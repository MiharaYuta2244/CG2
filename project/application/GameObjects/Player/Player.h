#pragma once
#include "AABB.h"
#include "DirectInput.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "PlayerHealth.h"
#include "PlayerMove.h"

class EnemyManager;
class Enemy;

/// <summary>
/// プレイヤーの処理をまとめたクラス
/// </summary>
class Player {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, DirectInput* input, GamePad* gamePad, EnemyManager* enemyManager);

	// 当たり判定などの解決後に行う最終更新処理
	void PostUpdate();

	// 描画処理
	void Draw();

	// 座標のGetter
	Vector3 GetPosition() const { return transform_.translate; }
	Vector3& GetPosition() { return transform_.translate; }

	// 座標のSetter
	void SetPosition(const Vector3& pos) { transform_.translate = pos; }

	// 死亡フラグGetter
	bool IsDead() const;

	// ダメージ処理
	void Damage(float value);

	// 攻撃用の当たり判定Getter
	AABB GetAttackCol() const { return attackCol_; }

	// 攻撃可能かどうかSetter
	void SetEnableAttack(bool enableAttack) { enableAttack_ = enableAttack; }

	// 本体の当たり判定
	AABB GetBodyCol() const { return bodyCol_; }

	Enemy* GetTargetEnemy() const { return heldEnemy_; }

	// 現在のHP取得Getter
	float GetCurrentHP() const { return hp_->GetCurrentHP(); }

	// HPの最大値取得Getter
	float GetMaxHP() const { return hp_->GetMaxHP(); }

	// 描画用のオブジェクトを返すGetter
	TinyEngine::Object3d* GetObject3d() { return render_->GetObject3d(); }

	// プレイヤーが移動中かどうかを取得するGetter
	bool IsMoving() const { return isMoving_; }

private:
	// 当たり判定の更新処理
	void UpdateCollision();

private:
	Transform transform_;
	Vector2 velocity_;
	AABB attackCol_;
	AABB bodyCol_;
	Vector2 lastMoveDirection_;

	// プレイヤーの最大HP
	float maxHP_ = 3.0f;

	// 攻撃可能かどうかを表す変数
	bool enableAttack_ = false;

	bool isHold_ = false;

	// つかんでいる敵のポインタを記憶するための変数
	Enemy* heldEnemy_ = nullptr;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
	std::unique_ptr<PlayerMove> move_;     // 移動用インスタンス
	std::unique_ptr<PlayerHealth> hp_;     // HP管理用インスタンス

	float envScale_ = 1.0f;

	// 現在移動中かどうかを保持するフラグ
	bool isMoving_ = false;

	// 掴める距離の閾値
	float grabRange_ = 3.0f;
};
