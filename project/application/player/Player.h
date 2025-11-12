#pragma once
#include "Actor.h"
#include "DirectInput.h"
#include "Sprite.h"

class Object3dCommon;
class SpriteCommon;
class TextureManager;
class ModelManager;
class GamePad;

class Player : public Actor {
public:
	void Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane, DirectInput* input, GamePad* gamePad, SpriteCommon* spriteCommon, std::string filePath);

	void Update(float deltaTime);

	void Draw();

	// ImGui
	void UpdateImGui();

	// Setter
	void SetModel(const std::string model) { object3d_->SetModel(model); }
	void SetTranslate(Vector3 translate) { transform_.translate = translate; }
	void SetVelocity(Vector2 velocity) { velocity_ = velocity; }
	void SetIsHitEnemy(bool isHpSub) { isHitEnemy_ = isHpSub; }
	void SetIsInvincible(bool isInvincible) { isInvincible_ = isInvincible; }
	void SetIsHitEnemyHipDrop(bool isHitEnemyHipDrop) { isHitEnemyHipDrop_ = isHitEnemyHipDrop; }

	// Getter
	Object3d* GetObject3d() { return object3d_.get(); }
	Transform GetTransform() { return transform_; }
	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }
	Vector2 GetVelocity() { return velocity_; }
	int GetIsHpSub() { return isHitEnemy_; }
	bool GetIsInvincible() { return isInvincible_; }
	bool GetIsHitEnemyHipDrop() { return isHitEnemyHipDrop_; }

private:
	enum class Direction {
		RIGHT = 0,
		LEFT = 1,
	};

private:
	// 横移動
	void HorizontalMove();

	// ジャンプ
	void Jump();

	// ヒップドロップ
	void HipDrop();

	// ヒップドロップアニメーション
	void AnimationHipDrop();

	// 当たり判定の位置更新
	void UpdateCollisionPos();

	// 敵との接触時の処理
	void HitEnemy();

	// 無敵状態フレームカウント
	void FrameCountIsInvincible();

	// ダメージ処理
	void SubHP();

	// ヒップドロップが敵に当たった時にプレイヤーを打ち上げる
	void AfterHipDrop();

private:
	// 入力
	DirectInput* input_ = nullptr;

	// ジャンプ中かどうか
	bool isJump_ = false;

	// ジャンプ力
	float jumpPower_ = 20.0f;

	// ヒップドロップ力
	float hipDropPower_ = -35.0f;

	// 速度関連
	Vector2 acceleration_ = {0.0f, -25.0f};
	Vector2 velocity_ = {8.0f, 0.0f};
	float gravity_ = 0.01f;

	// ゲームパッド
	GamePad* gamePad_ = nullptr;

	// 経過時間
	float deltaTime_;

	// ヒップドロップ時の回転フラグ
	bool isRotate_ = false;

	// 地面の高さ
	float groundPosY_ = 2.0f;

	// プレイヤーの向いている方向
	Direction direction_ = Direction::RIGHT;

	// HP
	int hp_ = 5;

	// 敵に当たった時に立つフラグ
	bool isHitEnemy_ = false;

	// 無敵状態フラグ
	bool isInvincible_ = false;

	// 無敵時間フレームカウント
	int invincibleFrameCount_ = 0;

	// 無敵時間上限
	const int kInvincibleFrame_ = 60;

	// HPゲージスプライト(ハート)
	std::unique_ptr<Sprite> spriteHeart_ = std::make_unique<Sprite>();

	// 敵にヒップドロップを当てた時に立つフラグ
	bool isHitEnemyHipDrop_ = false;
};
