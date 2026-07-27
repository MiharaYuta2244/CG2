#pragma once
#include "AABB.h"
#include "GameObjects/PlaceableObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"

/// <summary>
/// 檻クラス
/// </summary>
class Cage : public PlaceableObject {
public:
	Cage();

	// 初期化処理
	void Initialize(EngineContext* ctx, const Transform& transform);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw()override;

	// 当たり判定Getter
	AABB GetCollision() const { return collision_; }

	// ギズモ用
	std::string GetName() const override { return "Cage(" + std::to_string(id_) + ")"; }

	// アウトライン有効フラグSetter
	void SetEnableOutline(bool isEnable) override;

private:
	AABB collision_; // 当たり判定
	Vector4 color_ = {0.1f, 0.1f, 0.1f, 1.0f};
	float time_ = 0.0f;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス

	// オブジェクト数カウント用
	static int index;
	int id_ = 0;
};
