#include "EndModel.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <numbers>

void EndModel::Initialize(EngineContext* ctx) {
	endModel_ = std::make_unique<Object3d>();
	endModel_->Initialize(ctx);
	endModel_->SetModel("end.obj");
	endModel_->SetTranslate({20.0f, -4.0f, 0.0f});
	endModel_->SetScale(baseScale_);
	endModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void EndModel::Update(float deltaTime) {
	// 拡縮アニメーション
	Animation(deltaTime);

	if (!isSelected_) {
		endModel_->SetScale(baseScale_ * 0.8f); // 非選択状態のときは通常よりサイズを小さくする
		animationTime_ = 0.0f;                  // アニメーションタイムをリセット
	}

	endModel_->Update();
}

void EndModel::Draw() { endModel_->Draw(); }

void EndModel::Animation(float deltaTime) {
	if (!isSelected_)
		return;

	animationTime_ += deltaTime * animationSpeed_;

	// サイン波で拡縮を繰り返す（0～1を往復）
	float t = (sin(animationTime_) + 1.0f) * 0.5f;

	// 補間して新しいスケールを計算
	Vector3 newScale = MathUtility::Lerp(baseScale_, targetScale_, t);

	endModel_->SetScale(newScale);
}