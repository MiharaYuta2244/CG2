#pragma once
#include "AABB.h"
#include "AnimationBundle.h"
#include "EnemyAI.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "VisonCone.h"
#include "ExclamationMark.h"

class EnemyBulletManager;
class Player;
class WallManager;

class Enemy {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, Vector3 pos);

	// 更新処理
	void Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager);

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

	void SetEnableMove(bool enableMove) { enableMove_ = enableMove; }

	// ノックバックを始める
	void StartKnockBack(Vector3 dir);

	// ノックバック中かどうかを取得
	bool IsKnockBack() const { return knockBackAnim_.anim.GetIsActive(); }

	// 死亡させる処理
	void Kill();

	// TransformのGetter
	Transform GetTransform() const { return transform_; }

private:
	// 当たり判定の更新
	void UpdateCollision();

	// プレイヤー発見時「!」マークの生成
	void GenerateExMark();

private:
	Transform transform_;
	AABB bodyCol_; // 本体のAABB
	bool enableMove_ = true;
	AnimationBundle<Vector3> knockBackAnim_;
	bool isDead_ = false;

	// ノックバックの強さ
	float knockBackPower_ = 20.0f;

	std::unique_ptr<ObjectRender> render_;  // 描画用インスタンス
	std::unique_ptr<EnemyAI> ai_;           // AI
	std::unique_ptr<TinyEngine::VisionCone> visionCone_; // 視界
	std::unique_ptr<ExclamationMark> exclamationMark_; // 「!」マーク

	EngineContext* ctx_ = nullptr;

	// プレイヤーを発見したかどうか
	EnemyAI::State lastState = EnemyAI::State::Normal;
};
