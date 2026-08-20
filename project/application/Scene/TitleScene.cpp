#include "TitleScene.h"
#include "SceneManager.h"

using namespace TinyEngine;

void TitleScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();
	commonData_->killCount = 0;
	commonData_->clearTime = 0.0f;
	commonData_->currentStageNo = 1;
	commonData_->currentStageKey = "Stage1";

	// メニューの生成&初期化
	menu_ = std::make_unique<MenuList>();
	menu_->Initialize(ctx.engineContext);
	menu_->AddItem("Play", "Title_Play.png", [this]() { RequestSceneChange(commonData_->currentStageKey); });
	menu_->AddItem("Quit", "Title_Quit.png", [this]() {
		finishTimer_ = std::make_unique<GameTimer>();
		finishTimer_->Initialize(0.5f);
	});

	// タイトルロゴの生成&初期化
	titleLogo_ = std::make_unique<Sprite>();
	titleLogo_->Initialize(ctx.engineContext, "Title_Logo.png");
	titleLogo_->SetPosition(titleLogoPos_);

	// 背景の生成&初期化
	background_ = std::make_unique<Sprite>();
	background_->Initialize(ctx.engineContext, "white.png");
	background_->SetSize({1280.0f, 720.0f});
	background_->SetColor(backgroundColor_);
	background_->SetEnableVoronoi(true);
	background_->SetVoronoiColor(voronoiColor_);
	background_->SetZDepth(100.0f);

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
	audioManager_->LoadWave("TitleBGM", "resources/sounds/bgm/TitleScene.mp3");
	audioManager_->PlayBGM("TitleBGM");
}

void TitleScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

	// 音声更新
	audioManager_->Update();

	// メニューの更新
	menu_->Update(ctx_.keyboard, ctx_.gamePad, ctx_.timeManager->GetDeltaTime());

	// タイトルロゴ更新
	titleLogo_->SetPosition(titleLogoPos_);
	titleLogo_->SetColor(logoColor_);
	titleLogo_->Update();

	// 背景更新
	voronoiTimer_ += deltaTime;
	background_->SetVoronoiParams(voronoiParams_.x, voronoiParams_.y, voronoiParams_.z, voronoiTimer_);
	background_->SetColor(backgroundColor_);
	background_->SetVoronoiColor(voronoiColor_);
	background_->Update();

	// 歪みのパラメータにDeltaTime加算
	glitchParam_.time += deltaTime;
	scanlineParam_.time += deltaTime;

	// ゲーム終了タイマー更新
	if (finishTimer_) {
		finishTimer_->Update(deltaTime);

		if (finishTimer_->IsEnd()) {
			// ゲーム終了
			PostQuitMessage(0);
		}
	}

#ifdef USE_IMGUI
	ImGui::Begin("PostEffect");

	// =======================
	// Scanline
	// =======================
	if (ImGui::CollapsingHeader("Scanline")) {
		ImGui::DragFloat("ScanlineCount", &scanlineParam_.scanlineCount, 1.0f, 1.0f, 2000.0f);
		ImGui::DragFloat("Intensity", &scanlineParam_.intensity, 0.01f, 0.0f, 5.0f);
		ImGui::DragFloat("Speed", &scanlineParam_.speed, 0.01f, 0.0f, 5.0f);
	}

	// =======================
	// Fisheye
	// =======================
	if (ImGui::CollapsingHeader("Fisheye")) {
		ImGui::DragFloat("Strength", &barrelDistortionParam_.strength, 0.01f, 0.0f, 5.0f);
	}

	// =======================
	// Glitch
	// =======================
	if (ImGui::CollapsingHeader("Glitch")) {
		ImGui::DragFloat("Intensity", &glitchParam_.intensity, 0.01f, 0.0f, 5.0f);
	}

	ImGui::End();
#endif

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
		glitch->SetGlitchTime(glitchParam_.time);
		glitch->SetGlitchIntensity(glitchParam_.intensity);
	}
}

void TitleScene::Draw() {
	// 背景描画
	background_->Draw();

	// メニュー描画
	menu_->Draw();

	// タイトルロゴ描画
	titleLogo_->Draw();
}

void TitleScene::Finalize() { audioManager_->StopBGM(); }
