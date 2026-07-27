#pragma once
#include "Transform.h"
#include "AABB.h"

/// <summary>
/// ゲームオブジェクトの基底クラス
/// </summary>
class IGameObject {
public:
	virtual ~IGameObject() = default;

	Transform& GetTransform() { return transform_; }

	// ImGuiのリスト表示用に使用する名前
	virtual std::string GetName() const { return "GameObject"; }

	// ギズモ用の当たり判定
	virtual AABB GetAABBForGizmo() const { return aabbForGizmo_; }

	// アウトラインの有効/無効を切り替える仮想関数
	virtual void SetEnableOutline(bool isEnable) {}

protected:
	// ギズモ用の当たり判定の更新
	void UpdateAABBForGizmo();

protected:
	Transform transform_{};
	AABB aabbForGizmo_;
};