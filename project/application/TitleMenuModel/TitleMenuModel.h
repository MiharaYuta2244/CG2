#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class TitleMenuModel {
public:
	void Initialize(EngineContext* ctx, Vector3 pos);
	void Update(float deltaTime);
	void Draw();

	// Getter
	Vector3 GetTranslate() const { return stageModel_->GetTranslate(); }
	bool GetSelected() const { return isSelected_; }
	Material& GetMaterial() { return stageModel_->GetMaterial(); }
	Transform& GetTransform() { return stageModel_->GetTransform(); }

	// Setter
	void SetSelected(bool isSelected) { isSelected_ = isSelected; }
	void SetTranslate(Vector3 translate) { stageModel_->SetTranslate(translate); }
	void SetModel(std::string filepath) { stageModel_->SetModel(filepath); }
	void SetColor(Vector4 color) { stageModel_->SetColor(color); }
	void SetRotate(const Vector3& rotate) { stageModel_->SetRotate(rotate); }

private:
	void Animation(float deltaTime);

private:
	std::unique_ptr<Object3d> stageModel_;

	// 選択状態かどうか
	bool isSelected_ = false;

	float animationTime_ = 0.0f;
	float animationSpeed_ = 2.0f;                     // ゆっくり動かすための速度係数
	Vector3 baseScale_ = Vector3(10.0f, 10.0f, 10.0f);   // 初期スケール
	Vector3 targetScale_ = Vector3(12.0f, 12.0f, 12.0f); // 拡縮の最大値

	Vector4 baseColor_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	Vector4 notSelectColor_ = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
};
