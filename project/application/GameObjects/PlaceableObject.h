#pragma once
#include "IGameObject.h"
#include "EngineContext.h"

/// <summary>
/// ステージオブジェクト専用中間クラス
/// </summary>
class PlaceableObject : public IGameObject {
public:
	virtual void Draw() = 0;
};