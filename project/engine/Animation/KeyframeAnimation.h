#pragma once
#include "MathUtility.h"
#include "Quaternion.h"
#include "Vector3.h"
#include <ModelData.h>
#include <cassert>

template<typename tValue> struct Keyframe {
	float time;
	tValue value;
};

using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

struct NodeAnimation {
	std::vector<KeyframeVector3> translate;
	std::vector<KeyframeQuaternion> rotate;
	std::vector<KeyframeVector3> scale;
};

template<typename tValue> struct AnimationCurve {
	std::vector<Keyframe<tValue>> keyframes;
};

struct Animation {
	std::string name;                                    // アニメーションの名前
	float duration;                                      // アニメーション全体の尺
	std::map<std::string, NodeAnimation> nodeAnimations; // NodeAnimationの集合。Node名でひけるようにしておく
};

/// <summary>
/// アニメーションクラス
/// </summary>
class KeyframeAnimation {
public:
	// アニメーション解析
	Animation LoadAnimationFile(const std::string& filename);

	// アニメーションの更新
	Matrix4x4 UpdateAnimation(float deltaTime, Animation* animation, ModelData* modelData);

public:
	// 任意の時刻の値を取得する
	template<typename tValue, typename keyframeType> static tValue CalculateValue(const std::vector<keyframeType>& keyframes, float time) {
		assert(!keyframes.empty());
		if (keyframes.size() == 1 || time <= keyframes[0].time) {
			return keyframes[0].value;
		}

		for (size_t index = 0; index < keyframes.size() - 1; ++index) {
			size_t nextIndex = index + 1;

			// indexとnextIndexの2つのKeyframeを取得して範囲内に時刻があるかを判定
			if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
				// 範囲内を補間する
				float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
				return MathUtility::Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
			}
		}

		// ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
		return (*keyframes.rbegin()).value;
	}

private:
	// アニメーション用タイマー
	float animationTimer_ = 0.0f;
};
