#include "ObjectRender.h"

using namespace TinyEngine;

void ObjectRender::Initialize(EngineContext* ctx, const std::string& filepath) {
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(ctx, filepath);
	object3d_->SetModel(filepath);
}

void ObjectRender::Update(Transform transform) {
	object3d_->SetTransform(transform);
	object3d_->Update();
}

void ObjectRender::Update() {
	object3d_->Update();
}

void ObjectRender::Draw() { object3d_->Draw(); }
