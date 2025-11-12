#pragma once
#include "Actor.h"

class Object3dCommon;
class TextureManager;
class ModelManager;

class Enemy : public Actor {
public:
	void Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane);

	void Update(float deltaTime);

	void Draw();

	// ImGui
	void UpdateImGui();

	// Setter
	void SetModel(const std::string model) { object3d_->SetModel(model); }
	void SetIsHitPlayerHipDrop(bool isHitPlayerHipDrop) { isHitPlayerHipDrop_ = isHitPlayerHipDrop; }

	// Getter
	Object3d* GetObject3d() { return object3d_.get(); }
	Transform GetTransform() { return transform_; }
	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }
	AABB GetAABBRightSide() { return collisionRightSide_; }
	AABB GetAABBLeftSide() { return collisionLeftSide_; }

private:
	// 横移動
	void HorizontalMove();

	// 反転処理
	void DirectionChange();

	// 当たり判定の位置更新
	void UpdateCollisionPos();

	// プレイヤーからヒップドロップを受けた時の処理
	void HitPlayerHipDrop();

	// ダメージ処理
	void SubHP();

	// 無敵状態フレームカウント
	void FrameCountIsInvincible();

private:
	enum class Direction {
		RIGHT = 0,
		LEFT = 1,
	};

private:
	// 速度関連
	Vector2 acceleration_ = {0.0f, -25.0f};
	Vector2 velocity_ = {8.0f, 0.0f};

	// HP
	int hp_ = 10;

	// 経過時間
	float deltaTime_;

	// 方向
	Direction direction_ = Direction::RIGHT;

	// プレイヤーからヒップドロップを受けた時に立つフラグ
	bool isHitPlayerHipDrop_ = false;

	// 無敵状態フラグ
	bool isInvincible_ = false;

	// 無敵時間フレームカウント
	int invincibleFrameCount_ = 0;

	// 無敵時間上限
	const int kInvincibleFrame_ = 60;

	// 当たり判定　右側
	AABB collisionRightSide_;

	// 当たり判定　左側
	AABB collisionLeftSide_;
};
