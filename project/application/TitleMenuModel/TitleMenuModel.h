#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class TitleMenuModel {
public:
	void Initialize(EngineContext* ctx, Vector3 pos);
	void Update(float deltaTime);
	void Draw();

	// Setter
	void SetSelected(bool isSelected) { isSelected_ = isSelected; }
	void SetTranslate(Vector3 translate) { stageModel_->SetTranslate(translate); }
	void SetModel(std::string filepath) { stageModel_->SetModel(filepath); }
	void SetColor(Vector4 color) { stageModel_->SetColor(color); }

private:
	void Animation(float deltaTime);

private:
	std::unique_ptr<Object3d> stageModel_;

	// 選択状態かどうか
	bool isSelected_ = true;

	float animationTime_ = 0.0f;
	float animationSpeed_ = 2.0f;                     // ゆっくり動かすための速度係数
	Vector3 baseScale_ = Vector3(5.0f, 5.0f, 5.0f);   // 初期スケール
	Vector3 targetScale_ = Vector3(5.5f, 5.5f, 5.5f); // 拡縮の最大値
};
