#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class TitleText {
public:
	void Initialize(EngineContext* ctx);
	void Update(float deltaTime);
	void Draw();

	// Getter
	Vector3& GetTranslate() { return textModel_->GetTranslate(); }
	Vector3& GetRotate() { return textModel_->GetRotate(); }
	Vector3& GetScale() { return textModel_->GetScale(); }
	Material& GetMaterial() { return textModel_->GetMaterial(); }
	Vector4& GetColor() { return textModel_->GetColor(); }

	// Setter
	void SetScale(Vector3 scale) { textModel_->SetScale(scale); }

private:
	std::unique_ptr<Object3d> textModel_;

	float baseScale_ = 12.0f; // 基本の大きさ
	float amplitude_ = 10.0f; // 拡縮の幅
	float beatInterval_ = 10.0f;

	float timer_ = 0.0f;
	const float kTimer = 3.0f;
};
