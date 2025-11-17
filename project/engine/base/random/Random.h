#pragma once
#include <random>
#include <vector>

class RandomEngine {
public:
	// コンストラクタでシード指定可能
	explicit RandomEngine(unsigned int seed = std::random_device{}()) : gen(seed) {}

	// float乱数
	float RangeFloat(float min, float max);

	// int乱数
	int RangeInt(int min, int max);

	// bool乱数（確率指定）
	bool Chance(float probability);

	// 配列からランダムに選択
	template<typename T> const T& Choice(const std::vector<T>& values) {
		std::uniform_int_distribution<size_t> dist(0, values.size() - 1);
		return values[dist(gen)];
	}

	// シャッフル
	template<typename T> void Shuffle(std::vector<T>& values) { std::shuffle(values.begin(), values.end(), gen); }

private:
	std::mt19937 gen;
};

// ユーティリティ名前空間
namespace RandomUtils {
// グローバルに使う乱数エンジン（シードはランダム）
inline RandomEngine engine{};

inline float RangeFloat(float min, float max) { return engine.RangeFloat(min, max); }

inline int RangeInt(int min, int max) { return engine.RangeInt(min, max); }

inline bool Chance(float probability) { return engine.Chance(probability); }

template<typename T> inline const T& Choice(const std::vector<T>& values) { return engine.Choice(values); }

template<typename T> inline void Shuffle(std::vector<T>& values) { engine.Shuffle(values); }
} // namespace RandomUtils
