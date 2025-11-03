#pragma once
#include <chrono>

class DeltaTime {
public:
	void Update();

	float GetDeltaTime() const { return deltaTime_; }

private:
	std::chrono::high_resolution_clock::time_point lastTime_ = std::chrono::high_resolution_clock::now();
	float deltaTime_ = 0.0f;
};
