#pragma once
#include <GameObjects/ObjectRender/ObjectRender.h>

/// <summary>
/// 地面
/// </summary>
class Ground {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

private:
	std::unique_ptr<ObjectRender> render_;
	Transform transform_;
	Vector4 color_ = {0.0235f, 0.0392f, 0.0784f, 1.0f};
};
