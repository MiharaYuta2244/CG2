#include "ToTitleModel.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <numbers>

void ToTitleModel::Initialize(EngineContext* ctx) {
	toTitleModel_ = std::make_unique<Object3d>();
	toTitleModel_->Initialize(ctx);
	toTitleModel_->SetModel("ToTitle2.obj");
	toTitleModel_->SetTranslate({35.0f, 0.0f, 0.0f});
	toTitleModel_->SetScale(baseScale_);
	toTitleModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void ToTitleModel::Update(float deltaTime) {
	// 拡縮アニメーション
	Animation(deltaTime);

	if (!isSelected_) {
		toTitleModel_->SetScale(baseScale_ * 0.5f); // 非選択状態のときは通常よりサイズを小さくする
		animationTime_ = 0.0f;                      // アニメーションタイムをリセット
	}

	toTitleModel_->Update();
}

void ToTitleModel::Draw() { toTitleModel_->Draw(); }

void ToTitleModel::Animation(float deltaTime) {
	if (!isSelected_)
		return;

	animationTime_ += deltaTime * animationSpeed_;

	// サイン波で拡縮を繰り返す（0～1を往復）
	float t = (sin(animationTime_) + 1.0f) * 0.5f;

	// 補間して新しいスケールを計算
	Vector3 newScale = MathUtility::Lerp(baseScale_, targetScale_, t);

	toTitleModel_->SetScale(newScale);
}
