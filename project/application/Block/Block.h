#pragma once
#include "Actor.h"
#include "EngineContext.h"
#include "AnimationBundle.h"

class Block : public Actor{
public:
	void Initialize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

	void Spawn(Vector3 pos);

	// Setter
	void SetModel(const std::string model) { object3d_->SetModel(model); }

	// Getter
	Object3d* GetObject3d() { return object3d_.get(); }

private:
	// 上昇アニメーション
	void UpMoveAnimation(float deltaTime);

private:
	// コンテキスト構造体
	EngineContext* ctx_;

	// 上昇アニメーション
	AnimationBundle<float> upMoveAnimation_;
};
