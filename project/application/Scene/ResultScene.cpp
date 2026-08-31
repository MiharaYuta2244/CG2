#include "ResultScene.h"
#include "SceneManager.h"

using namespace TinyEngine;

void ResultScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();

	// UI管理クラス生成&初期化
	uiManager_ = std::make_unique<ResultUIManager>();
	uiManager_->Initialize(ctx.engineContext, commonData_);

	// コールバック登録
	uiManager_->GetRetryButton()->SetCallback([this]() { RequestSceneChange(commonData_->currentStageKey); });
	uiManager_->GetToTitleButton()->SetCallback([this]() { RequestSceneChange("Title"); });

	// 生成したボタンをリストに登録する
	uiManager_->RegisterButtons();

	// メインカメラ
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize();

	// シーンで使うエフェクトの宣言
	ctx_.engineContext->postEffectPipeline->SetEffects({
	    PostEffectType::Scanline,         // 走査線
	    PostEffectType::BarrelDistortion, // 魚眼
	    PostEffectType::Glitch,           // グリッチ
	});

	barrelDistortionParam_.strength = 0.05f;
	glitchParam_.intensity = 0.1f;
	scanlineParam_.scanlineCount = 150.0f;
	scanlineParam_.intensity = 0.7f;
	scanlineParam_.speed = 5.0f;

	// オーディオマネージャーの生成&初期化
	audioManager_ = std::make_unique<AudioManager>();
	audioManager_->Initialize();
	audioManager_->LoadWave("ResultBGM", "resources/sounds/bgm/ResultScene.mp3");
	audioManager_->PlayBGM("ResultBGM");

	// リザルトテキスト生成&初期化
	resultText_ = std::make_unique<Sprite>();

	if (commonData_->isClear) {
		resultText_->Initialize(ctx.engineContext, "Clear.png");
		resultText_->SetPosition({515, 100});
	} else {
		resultText_->Initialize(ctx.engineContext, "GameOver.png");
		resultText_->SetPosition({390, 100});
	}

	// スコア表示インスタンス生成&初期化
	scoreDisplay_ = std::make_unique<ResultScoreDisplay>();
	scoreDisplay_->Initialize(ctx.engineContext, commonData_->isClear, commonData_->clearTime, commonData_->killCount);
}

void ResultScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

	// 音声更新
	audioManager_->Update();

	// 歪みのパラメータにDeltaTime加算
	glitchParam_.time += deltaTime;
	scanlineParam_.time += deltaTime;

	// UI管理クラス更新
	uiManager_->Update(deltaTime, ctx_.keyboard, ctx_.gamePad);

	auto* scanline = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Scanline);
	if (scanline) {
		scanline->SetScanlineParam(scanlineParam_);
	}

	auto* barrelDistortionParam = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::BarrelDistortion);
	if (barrelDistortionParam) {
		barrelDistortionParam->SetFisheyeParam(barrelDistortionParam_);
	}

	auto* glitch = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Glitch);
	if (glitch) {
		bool shouldGlitch = commonData_->postEffectSettings.glitchEnabled;
		glitch->SetGlitchTime(glitchParam_.time);
		glitch->SetGlitchIntensity(shouldGlitch ? glitchParam_.intensity : 0.0f);
	}

	// リザルトテキスト更新
	resultText_->Update();

	// スコア表示更新
	scoreDisplay_->Update(deltaTime);
}

void ResultScene::Draw() {
	// リザルト
	resultText_->Draw();

	// スコア表示描画
	scoreDisplay_->Draw();

	// UI管理クラス描画
	uiManager_->Draw();
}

void ResultScene::Finalize() { audioManager_->StopBGM(); }
