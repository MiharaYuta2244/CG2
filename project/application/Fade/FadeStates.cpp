#include "FadeStates.h"
#include "FadeManager.h"
#include <algorithm>

void FadeStateFadeOut::Enter(FadeManager* manager) { manager->SetFadeTimer(0.0f); }

void FadeStateFadeOut::Update(FadeManager* manager, float deltaTime) {
	manager->AddFadeTimer(deltaTime);
	float t = manager->GetFadeDuration() > 0.0f ? std::clamp(manager->GetFadeTimer() / manager->GetFadeDuration(), 0.0f, 1.0f) : 1.0f;
	manager->SetFadeAlpha(t);
	if (t >= 1.0f) {
		manager->ChangeState(std::make_unique<FadeStateWaiting>());
	}
}

void FadeStateWaiting::Enter(FadeManager* manager) { manager->SetWaitingForSceneChange(true); }

void FadeStateFadeIn::Enter(FadeManager* manager) { manager->SetFadeTimer(0.0f); }

void FadeStateFadeIn::Update(FadeManager* manager, float deltaTime) {
	manager->AddFadeTimer(deltaTime);
	float t = manager->GetFadeDuration() > 0.0f ? std::clamp(manager->GetFadeTimer() / manager->GetFadeDuration(), 0.0f, 1.0f) : 1.0f;
	manager->SetFadeAlpha(1.0f - t);
	if (t >= 1.0f) {
		manager->SetFadeAlpha(0.0f);
		manager->ChangeState(std::make_unique<FadeStateNone>());
	}
}
