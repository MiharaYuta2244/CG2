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

	void Update(float deltaTime);

	void Draw();

	// ImGui
	void UpdateImGui();

	// Setter
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
	float jumpPower_ = 10.0f;

	// 速度関連
	Vector2 acceleration_ = {0.0f, -9.81f};
	Vector2 velocity_ = {5.0f, 0.0f};
	float gravity_ = 0.01f;

	// ゲームパッド
	GamePad* gamePad_ = nullptr;

	// 経過時間
	float deltaTime_;
};
