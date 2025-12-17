#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class EndModel {
public:
	void Initialize(EngineContext* ctx);
	void Update(float deltaTime);
	void Draw();

private:
	std::unique_ptr<Object3d> endModel_;
};