#include "TimeManager.h"

void TimeManager::Update() {
	using namespace std::chrono;
	auto now = high_resolution_clock::now();
	deltaTime_ = duration<float>(now - lastTime_).count();
	fps_ = 1.0f / deltaTime_;
	lastTime_ = now;
}