#pragma once
#include "IFadeState.h"

/// <summary>
/// フェードなし
/// </summary>
class FadeStateNone : public IFadeState{
public:
	void Update(FadeManager* manager, float deltaTime)override{}
};

/// <summary>
/// フェードアウト中
/// </summary>
class FadeStateFadeOut : public IFadeState{
public:
	void Enter(FadeManager* manager)override;
	void Update(FadeManager* manager, float deltaTime) override;
};

/// <summary>
/// シーンの切り替え待ち
/// </summary>
class FadeStateWaiting : public IFadeState{
public:
	void Enter(FadeManager* manager) override;
	void Update(FadeManager* manager, float deltaTime) override{}
};

/// <summary>
/// フェードイン中
/// </summary>
class FadeStateFadeIn : public IFadeState{
public:
	void Enter(FadeManager* manager) override;
	void Update(FadeManager* manager, float deltaTime) override;
};
