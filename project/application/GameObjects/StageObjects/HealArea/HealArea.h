#pragma once
#include "GameObjects/PlaceableObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"

class HealArea : public PlaceableObject {
public:
	HealArea();

	void Initialize(EngineContext* ctx, const Transform& transform);

	void Update(float deltaTime);

	void Draw();

	// 当たり判定Getter
	AABB GetCollision() const { return collision_; }

	// ギズモ用
	std::string GetName() const override { return "HealArea(" + std::to_string(id_) + ")"; }

	// アクティブフラグSetter
	void SetIsActive(bool isActive) { isActive_ = isActive; }

	// アクティブフラグGetter
	bool GetIsActive() const { return isActive_; }

	// アウトライン有効フラグSetter
	void SetEnableOutline(bool isEnable) override;

private:
	std::unique_ptr<ObjectRender> render_ = nullptr;
	AABB collision_{};
	bool isActive_ = true;
	Vector4 color_ = {0.0f, 1.0f, 0.0f, 0.1f};

	// オブジェクト数カウント用
	static int index;
	int id_ = 0;
};
