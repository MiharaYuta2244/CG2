#pragma once
#include "Fruit/IFruit.h"

class Grape : public IFruit {
public:
	void Initialize(EngineContext* ctx) override;
	void Update(float deltaTime) override;
	void Draw() override;
};