#include "ReStartModel.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <numbers>

void ReStartModel::Initialize(EngineContext* ctx) {
	reStartModel_ = std::make_unique<Object3d>();
	reStartModel_->Initialize(ctx);
	reStartModel_->SetModel("saikai.obj");
	reStartModel_->SetTranslate({5.0f, 0.0f, 0.0f});
	reStartModel_->SetScale(baseScale_);
	reStartModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void ReStartModel::Update(float deltaTime) {
	// 拡縮アニメーション
	Animation(deltaTime);

	if (!isSelected_) {
		reStartModel_->SetScale(baseScale_ * 0.5f); // 非選択状態のときは通常よりサイズを小さくする
		animationTime_ = 0.0f;                      // アニメーションタイムをリセット
	}

	reStartModel_->Update();
}

void ReStartModel::Draw() { reStartModel_->Draw(); }

void ReStartModel::Animation(float deltaTime) {
	if (!isSelected_)
		return;

	animationTime_ += deltaTime * animationSpeed_;

	// サイン波で拡縮を繰り返す（0～1を往復）
	float t = (sin(animationTime_) + 1.0f) * 0.5f;

	// 補間して新しいスケールを計算
	Vector3 newScale = MathUtility::Lerp(baseScale_, targetScale_, t);

	reStartModel_->SetScale(newScale);
}
