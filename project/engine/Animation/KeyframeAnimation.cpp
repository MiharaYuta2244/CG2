#include "KeyframeAnimation.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

KeyframeAnimation::Animation KeyframeAnimation::LoadAnimationFile(const std::string& filename) { 
	Animation animation;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename.c_str(), 0);
	assert(scene->mNumAnimations!=0); // アニメーションがない
	aiAnimation* animationAssimp = scene->mAnimations[0]; // 最初のアニメーションだけ採用。
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond); // 時間の単位を秒に変換

	// assimpでは個々のNodeのAnimationをchannelと呼んでいるのでchannelを回してNodeAnimationの情報を取ってくる
	for(uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex){
		aiNodeAnim* nodeAnimationAssimp=animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation=animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

		// Translate
		for(uint32_t keyIndex = 0; keyIndex<nodeAnimationAssimp->mNumPositionKeys; ++keyIndex){
			aiVectorKey& keyAssimp=nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z};
			nodeAnimation.translate.push_back(keyframe);
		}

		// Rotate
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = {keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w};
			nodeAnimation.rotate.push_back(keyframe);
		}

		// Scale
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = {keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z};
			nodeAnimation.scale.push_back(keyframe);
		}
	}

	// 解析完了
	return animation;
}

Matrix4x4 KeyframeAnimation::UpdateAnimation(float deltaTime, Animation* animation, ModelData* modelData) {
	animationTimer_+=deltaTime;
	animationTimer_=fmod(animationTimer_,animation->duration); // 最後までいったら最初からリピート再生
	NodeAnimation& rootNodeAnimation = animation->nodeAnimations[modelData->rootNode.name];
	Vector3 translate = CalculateValue<Vector3, KeyframeVector3>(rootNodeAnimation.translate, animationTimer_);
	Quaternion rotate = CalculateValue<Quaternion, KeyframeQuaternion>(rootNodeAnimation.rotate, animationTimer_);
	Vector3 scale = CalculateValue<Vector3, KeyframeVector3>(rootNodeAnimation.scale, animationTimer_);
	Matrix4x4 localMatrix = MathUtility::MakeAffineMatrix(scale, rotate, translate);

	return localMatrix;
}
