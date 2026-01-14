#pragma once
#include "Object3d.h"
#include "EngineContext.h"
#include <memory>

class GunTurret {
public:
	void Initialize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

private:
	// 砲台モデル
	std::unique_ptr<Object3d> gunTurretModel_;
};
