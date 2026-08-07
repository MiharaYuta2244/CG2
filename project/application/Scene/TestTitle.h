#pragma once
#include "BaseScene.h"
#include "Menu/MenuList.h"

class TestTitle : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// メニュー
	std::unique_ptr<MenuList> menu_;

	// タイトルロゴ
	std::unique_ptr<TinyEngine::Sprite> titleLogo_;
	Vector2 titleLogoPos_ = {0.0f, 100.0f};        // ロゴの座標
	Vector4 logoColor_ = {1.0f, 0.7f, 0.1f, 1.0f}; // 色

	// 背景
	std::unique_ptr<TinyEngine::Sprite> background_;
	Vector3 voronoiParams_ = {5.0f, 2.0f, 1.0f};         // ボロノイノイズのパラメータ
	float voronoiTimer_ = 0.0f;                          // ボロノイノイズ用タイマー
	Vector4 voronoiColor_ = {0.0f, 0.2f, 0.2f, 1.0f};    // ボロノイノイズの色
	Vector4 backgroundColor_ = {0.0f, 0.0f, 0.0f, 1.0f}; // 背景色

	// ポストエフェクトパラメータ
	ScanlineParam scanlineParam_;                 // 走査線
	BarrelDistortionParam barrelDistortionParam_; // 魚眼
	GlitchParam glitchParam_;                     // グリッチ
};
