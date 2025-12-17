#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class StartModel {
public:
	void Initialize(EngineContext* ctx);
	void Update(float deltaTime);
	void Draw();

private:
	std::unique_ptr<Object3d> startModel_;
};
