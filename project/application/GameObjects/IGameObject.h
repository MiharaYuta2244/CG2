#pragma once
#include "Transform.h"

/// <summary>
/// ゲームオブジェクトの基底クラス
/// </summary>
class IGameObject {
public:
	virtual ~IGameObject() = default;

	Transform& GetTransform() { return transform_; }

	// ImGuiのリスト表示用に使用する名前
	virtual std::string GetName() const { return "GameObject"; }

protected:
	Transform transform_{};
};