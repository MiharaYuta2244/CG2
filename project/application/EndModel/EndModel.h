#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class EndModel {
public:
	void Initialize(EngineContext* ctx);
	void Update(float deltaTime);
	void Draw();

	void SetSelected(bool isSelected) { isSelected_ = isSelected; }

private:
	void Animation(float deltaTime);

private:
	std::unique_ptr<Object3d> endModel_;

	// 選択状態かどうか
	bool isSelected_ = false;

	float animationTime_ = 0.0f;
	float animationSpeed_ = 2.0f;                     // ゆっくり動かすための速度係数
	Vector3 baseScale_ = Vector3(5.0f, 5.0f, 5.0f);   // 初期スケール
	Vector3 targetScale_ = Vector3(5.5f, 5.5f, 5.5f); // 拡縮の最大値
};