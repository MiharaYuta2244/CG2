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

	// Getter
	Object3d* GetObject3d() { return object3d_.get(); }
	Transform GetTransform() { return transform_; }
	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }

private:
	// 横移動
	void HorizontalMove();

private:
	// 速度関連
	Vector2 acceleration_ = {0.0f, -25.0f};
	Vector2 velocity_ = {8.0f, 0.0f};

	// HP
	int hp_ = 10;

	// 経過時間
	float deltaTime_;
};
