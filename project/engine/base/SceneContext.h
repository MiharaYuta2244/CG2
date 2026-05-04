#pragma once
#include "DebugCamera.h"
#include "DirectInput.h"
#include "EngineContext.h"
#include "GamePad.h"
#include "TimeManager.h"

class SceneManager;

struct SceneContext {
	EngineContext* engineContext = nullptr;
	DirectInput* keyboard = nullptr;
	GamePad* gamePad = nullptr;
	Camera* currentCamera = nullptr;
	TimeManager* timeManager = nullptr;
	SceneManager* sceneManager = nullptr;
};