#pragma once
#include "AABB.h"
#include "BloodDecalManager.h"
#include "DirectInput.h"
#include "GameObjects/IGameObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "GameTimer.h"
#include "OBB.h"
#include "Particle.h"
#include "PlayerHPIcon.h"
#include "PlayerHealth.h"
#include "PlayerMove.h"

class EnemyManager;
class Enemy;

/// <summary>
/// プレイヤーの処理をまとめたクラス
/// </summary>
class Player : public IGameObject {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, TinyEngine::BloodDecalManager* bloodDecalManager);

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

	// 回復処理
	void Heal(float value);

	// 全回復処理
	void AllHeal();

	// 攻撃用の当たり判定Getter
	OBB GetAttackCol() const { return attackCol_; }

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

	// 回転のGetter
	Vector3 GetRotation() const { return transform_.rotate; }

	// プレイヤーが移動中かどうかを取得するGetter
	bool IsMoving() const { return isMoving_; }

	// ギズモ用
	std::string GetName() const override { return "Player"; }

	// プレイヤーの向いている方向のGetter
	Vector2 GetDirection() const { return lastMoveDirection_; }

	// 敵を掴んでいるかどうかGetter
	bool IsGrabbingEnemy() const { return isGrabTriggered_; }

	void SetIsGrabReleased(bool isGrabReleased) { isGrabReleased_ = isGrabReleased; }

	void SetIsHold(bool isHold) { isHold_ = isHold; }

	// 掴んでいる敵のポインタGetter
	Enemy* GetGrabbedEnemy() { return heldEnemy_; }

	// 攻撃フラグGetter
	bool GetIsAttack() const { return isAttackTriggered_; }

private:
	// 当たり判定の更新処理
	void UpdateCollision();

	// ヒットエフェクト生成関数
	void GenerateHitEffect();

	// 回復エフェクトの生成関数
	void GenerateHealEffect();

	// 血痕の生成
	void AddBloodDecal(Vector3 scale);

	// HPが2以下の時の出血処理
	void Bleeding(float deltaTime);

private:
	Vector2 velocity_;
	OBB attackCol_;
	AABB bodyCol_;
	Vector2 lastMoveDirection_;
	EngineContext* ctx_ = nullptr;

	// 色
	Vector4 color_ = {1.0f, 0.1568f, 0.0f, 1.0f};

	// プレイヤーの最大HP
	float maxHP_ = 3.0f;

	// 攻撃可能かどうかを表す変数
	bool enableAttack_ = false;

	// 掴み状態かどうか
	bool isHold_ = false;

	// つかんでいる敵のポインタを記憶するための変数
	Enemy* heldEnemy_ = nullptr;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
	std::unique_ptr<PlayerMove> move_;     // 移動用インスタンス
	std::unique_ptr<PlayerHealth> hp_;     // HP管理用インスタンス
	std::unique_ptr<ObjectRender> hand_;   // 攻撃確認用
	std::unique_ptr<PlayerHPIcon> hpIcon_; // HPのUI

	// 環境マップ　強さ
	float envScale_ = 0.0f;

	// 現在移動中かどうかを保持するフラグ
	bool isMoving_ = false;

	// 掴める距離の閾値
	float grabRange_ = 3.0f;

	// 敵を掴んでいるときにかける速度倍率
	float speedMultiplier_ = 1.0f;

	// 攻撃用のオフセット
	float attackOffset_ = 1.5f;

	// パーティクル関連
	std::vector<std::unique_ptr<TinyEngine::Particle>> dustParticle_; // 砂埃パーティクル
	GameTimer particleGenerateTimer_;                                 // パーティクル生成用のタイマー

	// 被弾時エフェクト
	std::vector<std::unique_ptr<TinyEngine::Particle>> hitEffects_;
	float preHP_ = maxHP_;

	// 回復エフェクト
	std::vector<std::unique_ptr<TinyEngine::Particle>> healEffects_;

	// 攻撃確認用フラグ
	bool isHandAnimPlaying_ = false;

	// 敵を掴んでいるかどうか
	bool isGrabTriggered_ = false;

	// 攻撃しているかどうか
	bool isAttackTriggered_ = false;

	// 敵を放したかどうか
	bool isGrabReleased_ = false;

	// 血痕の管理インスタンスポインタ
	TinyEngine::BloodDecalManager* bloodDecalManager_ = nullptr;

	// 出血の間隔用タイマー
	GameTimer bleedingTimer_;

	// 出血の間隔
	float bleedingInterval_ = 0.2f;

	// プレイヤーの生存時間
	float aliveTImer_ = 0.0f;
};
