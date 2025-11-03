#include "DeltaTime.h"

void DeltaTime::Update() {
	using namespace std::chrono;
	auto now = high_resolution_clock::now();
	deltaTime_ = duration<float>(now - lastTime_).count();
	lastTime_ = now;
}