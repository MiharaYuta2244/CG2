#pragma once
#include "Actor.h"
#include "DirectInput.h"

class Object3dCommon;
class TextureManager;
class ModelManager;
class GamePad;

class Player : public Actor {
public:
	void Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane, DirectInput* input, GamePad* gamePad);

	void Update(float deltaTime);

	void Draw();

	// ImGui
	void UpdateImGui();

	// Setter
	void SetModel(const std::string model) { object3d_->SetModel(model); }

	// Getter
	Object3d* GetObject3d() { return object3d_.get(); }
	Transform GetTransform() { return transform_; }
	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }

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
};
