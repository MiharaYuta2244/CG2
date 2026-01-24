#include "StartModel.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <numbers>

void StartModel::Initialize(EngineContext* ctx) {
	startModel_ = std::make_unique<Object3d>();
	startModel_->Initialize(ctx);
	startModel_->SetModel("start.obj");
	startModel_->SetTranslate({32.0f, 3.35f, 0.0f});
	startModel_->SetScale(baseScale_);
	startModel_->SetRotate({0.0f, 3.6f, 0.0f});
	startModel_->SetEnableLighting(true);
}

void StartModel::Update(float deltaTime) {
	// 拡縮アニメーション
	Animation(deltaTime);

	if (!isSelected_) {
		startModel_->SetScale(baseScale_ * 0.5f); // 非選択状態のときは通常よりサイズを小さくする
		animationTime_ = 0.0f;                    // アニメーションタイムをリセット
	}

	startModel_->Update();
}

void StartModel::Draw() { startModel_->Draw(); }

void StartModel::Animation(float deltaTime) {
	if (!isSelected_)
		return;

	animationTime_ += deltaTime * animationSpeed_;

	// サイン波で拡縮を繰り返す（0～1を往復）
	float t = (sin(animationTime_) + 1.0f) * 0.5f;

	// 補間して新しいスケールを計算
	Vector3 newScale = MathUtility::Lerp(baseScale_, targetScale_, t);

	startModel_->SetScale(newScale);
}
