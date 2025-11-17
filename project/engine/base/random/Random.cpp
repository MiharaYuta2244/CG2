#include "Random.h"

float RandomEngine::RangeFloat(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(gen);
}

int RandomEngine::RangeInt(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(gen);
}

bool RandomEngine::Chance(float probability) {
	std::bernoulli_distribution dist(probability);
	return dist(gen);
}
