#pragma once
#include "AABB.h"
#include "AnimationBundle.h"
#include "EnemyAI.h"
#include "EnemyType.h"
#include "ExclamationMark.h"
#include "GameObjects/IGameObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "Particle.h"
#include "VisionCone.h"
#include "BloodDecalManager.h"

class EnemyBulletManager;
class Player;
class WallManager;
class DoorManager;

/// <summary>
/// 敵クラス
/// </summary>
class Enemy : public IGameObject {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, Vector3 pos, EnemyType type, TinyEngine::BloodDecalManager* bloodDecalManager);

	// 更新処理
	void Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager, DoorManager* doorManager, GlassManager* glassManager);

	// 押し戻し完了後に呼ぶ最終更新処理
	void PostUpdate();

	// 描画処理
	void Draw();

	// 死亡フラグGetter
	bool IsDead() const { return isDead_; }

	// 敵の当たり判定
	AABB GetBodyCol() const { return bodyCol_; }

	// 敵の座標Getter
	Vector3& GetPos() { return transform_.translate; }

	// 敵の座標Setter
	void SetPos(Vector3 pos) { transform_.translate = pos; }

	// 敵の回転Getter
	Vector3& GetRotate() { return transform_.rotate; }

	// 敵の回転Setter
	void SetRotate(Vector3 rot) { transform_.rotate = rot; }

	void SetEnableAI(bool enableMove) { enableMove_ = enableMove; }

	// ノックバックを始める
	void StartKnockBack(Vector3 dir);

	// ノックバック中かどうかを取得
	bool IsKnockBack() const { return knockBackAnim_.anim.GetIsActive(); }

	// 死亡させる処理
	void Kill();

	// ダメージ処理
	void Damage();

	// shotTimerリセット
	void ResetShotTimer() { ai_->ResetShotTimer(); }

	// TransformのGetter
	Transform GetTransform() const { return transform_; }

	// ギズモ用
	std::string GetName() const override { return "Enemy"; }

	// 無敵状態かどうか
	bool IsInvincible() const { return invincibleTimer_ > 0.0f; }

	void SetIsMove(bool isMove) { isMove_ = isMove; }
	bool GetIsMove() { return isMove_; }
	void SetShotHoldState(bool isShotHoldState) { ai_->SetShotHoldState(isShotHoldState); }
	void SetAIState(EnemyAI::State state);

	// アニメーションを止める処理
	void StopAnimation();

private:
	// 当たり判定の更新
	void UpdateCollision();

	// プレイヤー発見時「!」マークの生成
	void GenerateExMark();

	// マズルフラッシュ生成関数
	void GenerateMuzzleFlash(const Vector3& direction);

	// 血痕の生成
	void AddBloodDecal();

private:
	AABB bodyCol_; // 本体のAABB
	bool enableMove_ = true;
	AnimationBundle<Vector3> knockBackAnim_;
	bool isDead_ = false;

	// ノックバックの強さ
	float knockBackPower_ = 20.0f;

	std::unique_ptr<ObjectRender> render_;               // 描画用インスタンス
	std::unique_ptr<EnemyAI> ai_;                        // AI
	std::unique_ptr<TinyEngine::VisionCone> visionCone_; // 視界
	std::unique_ptr<ExclamationMark> exclamationMark_;   // 「!」マーク

	EngineContext* ctx_ = nullptr;

	// プレイヤーを発見したかどうか
	EnemyAI::State lastState = EnemyAI::State::Normal;

	// 環境マップ　強さ
	float envScale_ = 0.0f;

	// マズルフラッシュエフェクト管理用コンテナ
	std::vector<std::unique_ptr<TinyEngine::Particle>> muzzleParticles_;

	// チャージパーティクル
	std::unique_ptr<TinyEngine::Particle> chargeParticle_;
	std::unique_ptr<TinyEngine::Particle> chargeCylinderParticle_;

	// 移動フラグ
	bool isMove_ = true;

	// 敵のタイプ
	EnemyType type_;

	// HP
	int hp_ = 1;

	// 色
	Vector4 color_ = {1, 1, 1, 1};

	// 点滅用変数
	float damageBlinkTimer_ = 0.0f;
	bool isBlinkVisible_ = true;

	// 無敵時間用タイマー
	float invincibleTimer_ = 0.0f;

	// 死亡時の血痕
	TinyEngine::BloodDecalManager* bloodDecalManager_ = nullptr;
};
