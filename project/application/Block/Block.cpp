#include "Block.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "TextureManager.h"

void Block::Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane) {
	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();

	// Object3dの初期化
	object3d_->Initialize(obj3dCommon, texMane, ModelMane);

	transform_.translate = {0.0f, 0.0f, 0.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.scale = {1.0f, 1.0f, 1.0f};
}

void Block::Update() {
	object3d_->SetTransform(transform_);
	object3d_->Update();
}

void Block::Draw() { object3d_->Draw(); }

void Block::Spawn(Vector3 pos) { transform_.translate = pos; }
