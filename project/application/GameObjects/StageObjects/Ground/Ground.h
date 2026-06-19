#pragma once
#include <GameObjects/ObjectRender/ObjectRender.h>
#include "GameObjects/IGameObject.h"

/// <summary>
/// 地面
/// </summary>
class Ground : public IGameObject {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// ギズモ用
	std::string GetName() const override { return "Ground"; }

private:
	std::unique_ptr<ObjectRender> render_;
	Vector4 color_ = {0.0235f, 0.0392f, 0.0784f, 1.0f};
};
