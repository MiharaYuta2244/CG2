#include "TitleMenuModel.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <numbers>

void TitleMenuModel::Initialize(EngineContext* ctx, Vector3 pos) {
	stageModel_ = std::make_unique<Object3d>();
	stageModel_->Initialize(ctx);
	stageModel_->SetModel("start.obj");
	stageModel_->SetTranslate(pos);
	stageModel_->SetScale(baseScale_);
	stageModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
	stageModel_->SetColor(notSelectColor_);
}

void TitleMenuModel::Update(float deltaTime) {
	// 拡縮アニメーション
	Animation(deltaTime);

	if (!isSelected_) {
		stageModel_->SetScale(baseScale_ * 0.5f); // 非選択状態のときは通常よりサイズを小さくする
		animationTime_ = 0.0f;                    // アニメーションタイムをリセット
	}

	stageModel_->Update();
}

void TitleMenuModel::Draw() { stageModel_->Draw(); }

void TitleMenuModel::Animation(float deltaTime) {
	if (stageModel_->GetColor().x == notSelectColor_.x && isSelected_) {
		stageModel_->SetColor(baseColor_);
	} else if (stageModel_->GetColor().x == 1.0f && !isSelected_) {
		stageModel_->SetColor(notSelectColor_);
	}

	if (isSelected_) {
		animationTime_ += deltaTime * animationSpeed_;

		// サイン波で拡縮を繰り返す（0～1を往復）
		float t = (sin(animationTime_) + 1.0f) * 0.5f;

		// 補間して新しいスケールを計算
		Vector3 newScale = MathUtility::Lerp(baseScale_, targetScale_, t);

		stageModel_->SetScale(newScale);
	}
}
