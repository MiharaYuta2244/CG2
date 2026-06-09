#pragma once
#include "Sprite.h"

class Controls {
public:
	void Initialize(EngineContext* ctx);

	void Update();

	void Draw();

private:
	// 掴むUI
	std::unique_ptr<TinyEngine::Sprite> holdUI_;

	// 突きとばすUI
	std::unique_ptr<TinyEngine::Sprite> pushUI_;
};
