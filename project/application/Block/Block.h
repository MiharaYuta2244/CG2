#pragma once
#include "Actor.h"

class Object3dCommon;
class TextureManager;
class ModelManager;

class Block : public Actor{
public:
	void Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane);

	void Update();

	void Draw();

	void Spawn(Vector3 pos);

	// Setter
	void SetModel(const std::string model) { object3d_->SetModel(model); }

	// Getter
	Object3d* GetObject3d() { return object3d_.get(); }
};
