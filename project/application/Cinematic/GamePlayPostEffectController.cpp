#include "GamePlayPostEffectController.h"
#include "SceneManager.h"

using namespace TinyEngine;

void GamePlayPostEffectController::Initialize(const SceneContext& ctx) {
	ctx_ = &ctx;

	// シーンで使うエフェクトの宣言
	ctx_->engineContext->postEffectPipeline->SetEffects({
	    PostEffectType::Vignette,    // ビネット
	    PostEffectType::Glitch,      // グリッチ
	    PostEffectType::DeathEffect, // 死亡時エフェクト
	    PostEffectType::Smoothing,   // スムージング
	    PostEffectType::Gaussian,    // ガウシアン
	    PostEffectType::RadialBlur,  // ラディアルブラー
	});

	// ビネットの初期設定
	auto* vignette = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::Vignette);
	if (vignette) {
		vignette->SetVignetteColor({1, 0, 0, 1});
	}

	glitchTimer_ = 0.0f;
	elapsedTime_ = 0.0f;
	damageBlurTimer_ = 0.0f;
	isDeathAnimStarted_ = false;
	isAnimPlaying_ = false;
}

void GamePlayPostEffectController::Update(float deltaTime, float playerHP, bool isPlayerDead) {
	// グリッチノイズの更新
	UpdateGlitch(deltaTime);

	// ダメージブラータイマーの減算
	if (damageBlurTimer_ > 0.0f) {
		damageBlurTimer_ -= deltaTime;
		if (damageBlurTimer_ < 0.0f) {
			damageBlurTimer_ = 0.0f;
		}
	}

	// 死亡時アニメーションの更新
	isAnimPlaying_ = numSamplesAnim_.anim.Update(deltaTime, numSamplesAnim_.temp);

	// 各エフェクトパスの取得
	auto* vignette = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::Vignette);
	auto* smoothing = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::Smoothing);
	auto* gaussian = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::Gaussian);
	auto* radialBlur = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::RadialBlur);
	auto* deathEffect = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::DeathEffect);

	if (!isPlayerDead) {
		// 生存時のエフェクト更新
		if (vignette) {
			float intensity = playerHP <= 1.0f ? 0.5f : 0.0f;
			vignette->SetVignetteIntensity(intensity);
		}
		if (radialBlur) {
			RadialBlurParam param;
			param.blurWidth = playerHP <= 1.0f ? 0.01f : 0.0f;
			param.numSamples = playerHP <= 1.0f ? 5.0f : 1.0f;
			radialBlur->SetRadialBlurParam(param);
		}
		if (smoothing) {
			SmoothingParam param;
			param.radius = (damageBlurTimer_ > 0.0f) ? 2 : 0;
			smoothing->SetSmoothingParam(param);
		}
		if (gaussian) {
			GaussianParam param;
			param.radius = (damageBlurTimer_ > 0.0f) ? 2 : 0;
			gaussian->SetGaussianParam(param);
		}
		if (deathEffect) {
			deathEffect->SetDeathEffectIntensity(0.0f);
		}
	} else {
		// 死亡時のエフェクト更新
		if (vignette) {
			vignette->SetVignetteIntensity(0.0f);
		}
		if (deathEffect) {
			deathEffect->SetDeathEffectIntensity(1.0f);
		}
	}
}

void GamePlayPostEffectController::StartDeathAnimation() {
	if (!isDeathAnimStarted_) {
		isDeathAnimStarted_ = true;
		numSamplesAnim_.anim.Start(0.0f, 1.0f, 1.0f, EaseType::EASEOUTCIRC);
	}
}

void GamePlayPostEffectController::UpdateGlitch(float deltaTime) {
	if (glitchTimer_ > 0.0f) {
		glitchTimer_ -= deltaTime;
		elapsedTime_ += deltaTime;
		if (glitchTimer_ < 0.0f) {
			glitchTimer_ = 0.0f;
			elapsedTime_ = 0.0f;
		}
	}

	float intensity = (glitchTimer_ > 0.0f) ? 1.0f : 0.0f;
	auto* glitch = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::Glitch);
	if (glitch) {
		glitch->SetGlitchTime(elapsedTime_);
		glitch->SetGlitchIntensity(intensity);
	}
}

void GamePlayPostEffectController::Finalize() {
	// シーン終了時にエフェクトをデフォルトに戻す
	ctx_->engineContext->postEffectPipeline->SetEffects({PostEffectType::FullScreen});
}