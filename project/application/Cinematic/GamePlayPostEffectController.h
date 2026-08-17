#pragma once
#include "AnimationBundle.h"
#include "Matrix4x4.h"

// シーンコンテキスト等の前方宣言
struct SceneContext;

/// <summary>
/// ゲームプレイ中のポストエフェクト演出を管理するクラス
/// </summary>
class GamePlayPostEffectController {
public:
	void Initialize(const SceneContext& ctx);

	// 更新処理
	void Update(float deltaTime, float playerHP, bool isPlayerDead, const Matrix4x4& projectionInverse);

	void Finalize();

	// 死亡演出の開始
	void StartDeathAnimation();

	// 死亡演出が再生中かどうか
	bool GetIsDeathAnimPlaying() const { return isAnimPlaying_; }

	// 当たり判定マネージャに渡すためのタイマー参照
	float& GetGlitchTimer() { return glitchTimer_; }
	float& GetDamageBlurTimer() { return damageBlurTimer_; }

private:
	void UpdateGlitch(float deltaTime);

private:
	const SceneContext* ctx_ = nullptr;

	// グリッチノイズ用変数
	float glitchTimer_ = 0.0f;
	float elapsedTime_ = 0.0f;
	const float kGlitchDuration = 0.1f;

	// ダメージブラー用タイマー
	float damageBlurTimer_ = 0.0f;

	// プレイヤー死亡時演出用RadialBlurのNumSamplesAnim
	AnimationBundle<float> numSamplesAnim_;
	bool isDeathAnimStarted_ = false;
	bool isAnimPlaying_ = false;
};