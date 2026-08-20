#include "StageSelectScene.h"
#include "SceneManager.h"

using namespace TinyEngine;

void StageSelectScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();

	// 背景の生成&初期化
	background_ = std::make_unique<Sprite>();
	background_->Initialize(ctx.engineContext, "white.png");
	background_->SetSize({1280.0f, 720.0f});
	background_->SetColor({0.1f, 0.1f, 0.2f, 1.0f});

	// メニューの生成&初期化
	menu_ = std::make_unique<MenuList>();
	menu_->Initialize(ctx.engineContext);
	menu_->SetStartPos({520.0f, 300.0f});

	// Stage1
	menu_->AddItem("Stage1", "Stage1_Button.png", [this]() {
		commonData_->currentStageNo = 1;
		commonData_->currentStageKey = "Stage1";
		RequestSceneChange("Stage1");
	});

	// Stage2
	menu_->AddItem("Stage2", "Stage2_Button.png", [this]() {
		commonData_->currentStageNo = 2;
		commonData_->currentStageKey = "Stage2";
		RequestSceneChange("Stage2");
	});

	// タイトルに戻るボタン
	menu_->AddItem("BacktoTitle", "Title.png", [this]() { RequestSceneChange("Title"); });

	// ステージセレクトテキスト
	for (auto& text : stageSelectText_) {
		text = std::make_unique<Sprite>();
		text->Initialize(ctx.engineContext, "StageSelect.png");
	}

	stageSelectText_[0]->SetPosition({0, 0});
	stageSelectText_[1]->SetAnchorPoint({0.5f, 0.5f});
	stageSelectText_[1]->SetPosition({640, 680});
	stageSelectText_[1]->SetRotation(std::numbers::pi_v<float>);

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
}

void StageSelectScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

	// 背景更新
	background_->Update();

	// メニューの更新
	menu_->Update(ctx_.keyboard, ctx_.gamePad, ctx_.timeManager->GetDeltaTime());

	// UVTranslateのXを少しずづずらす
	stageSelectUvTranslate_.x += uvMoveSpeed_ * deltaTime;

	// ステージセレクトテキスト更新
	for (auto& text : stageSelectText_) {
		text->Update();
		text->SetUVTranslate(stageSelectUvTranslate_);
	}

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

void StageSelectScene::Draw() {
	// 背景描画
	background_->Draw();

	// メニュー描画
	menu_->Draw();

	// ステージセレクトテキスト描画
	for (auto& text : stageSelectText_) {
		text->Draw();
	}
}

void StageSelectScene::Finalize() {}