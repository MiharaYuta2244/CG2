#include "ResultScene.h"
#include "SceneManager.h"

using namespace TinyEngine;

void ResultScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();

	totalDamageLabel_ = std::make_unique<Sprite>();
	totalDamageLabel_->Initialize(ctx.engineContext, "DamageCount.png");
	totalDamageLabel_->SetPosition({300.0f, 200.0f});

	CreateNumberSprites(commonData_->totalDamageCount, {600.0f, 200.0f}, 100.0f, ctx, totalDamageNumbers_);

	totalDeathLabel_ = std::make_unique<Sprite>();
	totalDeathLabel_->Initialize(ctx.engineContext, "DeathCount.png");
	totalDeathLabel_->SetPosition({300.0f, 350.0f});

	CreateNumberSprites(commonData_->totalDeathCount, {600.0f, 350.0f}, 100.0f, ctx, totalDeathNumbers_);

	// UI管理クラス生成&初期化
	uiManager_ = std::make_unique<ResultUIManager>();
	uiManager_->Initialize(ctx.engineContext, commonData_);

	// コールバック登録
	uiManager_->GetRetryButton()->SetCallback([this]() { RequestSceneChange("PlayTest"); });
	uiManager_->GetToTitleButton()->SetCallback([this]() { RequestSceneChange("TestTitle"); });

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
}

void ResultScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

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
		glitch->SetGlitchTime(glitchParam_.time);
		glitch->SetGlitchIntensity(glitchParam_.intensity);
	}

	totalDamageLabel_->Update();
	for (auto& sprite : totalDamageNumbers_) {
		sprite->Update();
	}

	totalDeathLabel_->Update();
	for (auto& sprite : totalDeathNumbers_) {
		sprite->Update();
	}
}

void ResultScene::Draw() {
	// UI管理クラス描画
	uiManager_->Draw();

	totalDamageLabel_->Draw();
	for (auto& sprite : totalDamageNumbers_) {
		sprite->Draw();
	}

	totalDeathLabel_->Draw();
	for (auto& sprite : totalDeathNumbers_) {
		sprite->Draw();
	}
}

void ResultScene::Finalize() { 
	totalDamageNumbers_.clear(); 
	totalDeathNumbers_.clear();
}


void ResultScene::CreateNumberSprites(int value, const Vector2& startPos, float spacing, const SceneContext& ctx, std::vector<std::unique_ptr<TinyEngine::Sprite>>& outSprites) {
	std::string str = std::to_string(value);
	for (size_t i = 0; i < str.length(); ++i) {
		auto sprite = std::make_unique<Sprite>();
		std::string texName = std::string(1, str[i]) + ".png";

		sprite->Initialize(ctx.engineContext, texName);
		sprite->SetPosition({startPos.x + (i * spacing), startPos.y});
		outSprites.push_back(std::move(sprite));
	}
}