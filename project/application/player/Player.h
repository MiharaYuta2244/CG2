#pragma once
#include "DirectInput.h"
#include "Actor.h"

class Object3dCommon;
class TextureManager;
class ModelManager;
class GamePad;

class Player : public Actor {
public:
	void Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane, DirectInput* input, GamePad* gamePad);

	void Update();

	void Draw();

	void SetModel(const std::string model) { object3d_->SetModel(model); }

	// Getter
	Object3d* GetObject3d() { return object3d_.get(); }

private:
	// 横移動
	void HorizontalMove();

	// ジャンプ
	void Jump();

private:
	// 入力
	DirectInput* input_ = nullptr;

	// ジャンプ中かどうか
	bool isJump_ = false;

	// ジャンプ力
	float jumpPower_ = 0.2f;

	// 速度関連
	Vector3 acceleration_ = {0.0f, -0.098f, 0.0f};
	Vector3 velocity_ = {0.0f, 0.0f, 0.0f};
	float gravity_ = 0.01f;

	// ゲームパッド
	GamePad* gamePad_ = nullptr;
};
