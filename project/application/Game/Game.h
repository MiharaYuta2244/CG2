#pragma once
#include "Framework.h"
#include "SceneManager.h"
#include "Sprite.h"
#include <memory>

class Game : public Framework {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// フェード状態
	enum class FadeState {
		None = 0,
		FadeOut,
		FadeIn, 
		WaitingForSceneChange,
	};

	// フェード用スプライト
	std::unique_ptr<Sprite> fadeSprite_;

	// フェード管理
	FadeState fadeState_ = FadeState::None;
	float fadeDuration_ = 1.0f; // フェード時間（秒）
	float fadeTimer_ = 0.0f;

	// シーンマネージャー
	std::unique_ptr<SceneManager> sceneManager_ = std::make_unique<SceneManager>();

	// 現在のシーン名を記録（変更検知用）
	std::string lastSceneName_;

	// シーン切り替え要求
	std::string requestedSceneName_;
	bool isSceneChangeRequested_ = false;
};