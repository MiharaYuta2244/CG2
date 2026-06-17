#include "TitleScene.h"
#include "SceneManager.h"

void TitleScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();
	commonData_->killCount = 0;
	commonData_->clearTime = 0.0f;

	// メニューの生成&初期化
	menu_ = std::make_unique<MenuList>();
	menu_->Initialize(ctx.engineContext);
	menu_->AddItem("Play", "Title_Play.png", [this]() { RequestSceneChange("GamePlay"); });
	menu_->AddItem("Option", "Title_Option.png", [this]() { RequestSceneChange("GamePlay"); });
	menu_->AddItem("Quit", "Title_Quit.png", [this]() { PostQuitMessage(0); });

	// シーンで使うエフェクトの宣言
	ctx_.engineContext->postEffectPipeline->SetEffects({
	    PostEffectType::Scanline,   // 走査線
	    PostEffectType::BarrelDistortion,    // 魚眼
	    PostEffectType::Glitch,     // グリッチ
	});

	fisheyeParam_.strength = 0.05f;
	glitchParam_.intensity = 0.1f;
	scanlineParam_.scanlineCount = 150.0f;
	scanlineParam_.intensity = 0.7f;
	scanlineParam_.speed = 5.0f;
}

void TitleScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

	// メニューの更新
	menu_->Update(ctx_.keyboard, ctx_.gamePad, ctx_.timeManager->GetDeltaTime());

	// 歪みのパラメータにDeltaTime加算
	distortionParam_.time += deltaTime;
	glitchParam_.time += deltaTime;
	scanlineParam_.time += deltaTime;

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
		ImGui::DragFloat("Strength", &fisheyeParam_.strength, 0.01f, 0.0f, 5.0f);
	}

	// =======================
	// Glitch
	// =======================
	if (ImGui::CollapsingHeader("Glitch")) {
		ImGui::DragFloat("Intensity", &glitchParam_.intensity, 0.01f, 0.0f, 5.0f);
	}

	ImGui::End();

	auto* scanline = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Scanline);
	if (scanline) {
		scanline->SetScanlineParam(scanlineParam_);
	}

	auto* fisheye = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::BarrelDistortion);
	if (fisheye) {
		fisheye->SetFisheyeParam(fisheyeParam_);
	}

	auto* glitch = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Glitch);
	if (glitch) {
		glitch->SetGlitchTime(glitchParam_.time);
		glitch->SetGlitchIntensity(glitchParam_.intensity);
	}
#endif
}

void TitleScene::Draw() {
	// メニューの描画
	menu_->Draw();
}

void TitleScene::Finalize() {}
