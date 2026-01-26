#include "Block.h"

void Block::Initialize(EngineContext* ctx) {
	// コンテキスト構造体
	ctx_ = ctx;

	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();

	// Object3dの初期化
	object3d_->Initialize(ctx_);

	transform_.translate = {0.0f, 0.0f, 0.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.scale = {1.0f, 1.0f, 1.0f};

	// アニメーションの初期設定
	upMoveAnimation_.anim = {transform_.translate.y, 5.0f, 3.0f, EaseType::EASEOUTBOUNCE};
	upMoveAnimation_.temp = transform_.translate.y;
}

void Block::Update(float deltaTime) {
	// 上昇アニメーション
	//UpMoveAnimation(deltaTime);

	object3d_->SetTransform(transform_);
	object3d_->Update();
}

void Block::Draw() { object3d_->Draw(); }

void Block::Spawn(Vector3 pos) { transform_.translate = pos; }

void Block::UpMoveAnimation(float deltaTime) {
	if (upMoveAnimation_.anim.GetIsActive()) {
		upMoveAnimation_.anim.Update(deltaTime, upMoveAnimation_.temp);
		transform_.translate.y = upMoveAnimation_.temp;
	}
}
