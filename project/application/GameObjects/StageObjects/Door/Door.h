#pragma once
#include "GameObjects/ObjectRender/ObjectRender.h"

/// <summary>
/// ステージに配置するドア
/// </summary>
class Door {
	void Initialize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

private:
	// 描画用インスタンス
	std::unique_ptr<ObjectRender> render_;
};
