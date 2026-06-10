#pragma once
#include "AABB.h"
#include "GameObjects/ObjectRender/ObjectRender.h"

/// <summary>
/// ゴール判定用クラス
/// </summary>
class Goal {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// 当たり判定Getter
	AABB GetCol() const { return col_; }

	// ゴールフラグSetter
	void SetGoal(bool isGoal) { isGoal_ = isGoal; }

	// ゴールフラグGetter
	bool GetGoal() const { return isGoal_; }

private:
	Transform transform_;
	AABB col_;
	bool isGoal_ = false; // ゴールフラグ

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
};
