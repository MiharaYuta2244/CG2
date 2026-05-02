#pragma once
#include "DirectInput.h"
#include <Sprite.h>
#include <functional>
#include <string>

struct MenuItem {
	std::string label;
	std::function<void()> onSelect;
	std::unique_ptr<TinyEngine::Sprite> sprite;
	Vector2 originalSize;
};

/// <summary>
/// メニュー用UIクラス
/// </summary>
class MenuList {
public:
	// 初期化
	void Initialize(EngineContext* ctx);

	// メニューの追加
	void AddItem(const std::string& label, const std::string& texturePath, std::function<void()> onSelect);

	// 更新処理
	void Update(DirectInput* input, float deltaTime);

	// 描画処理
	void Draw();

private:
	EngineContext* ctx_ = nullptr;

	// メニューのリスト
	std::vector<MenuItem> items_;

	// 現在のインデックス
	int currentIndex_ = 0;

	// == タイトルロゴ ==
	std::unique_ptr<TinyEngine::Sprite> titleLogo_;
	Vector2 titleLogoPos_ = {0.0f, 100.0f};        // ロゴの座標
	Vector4 logoColor_ = {1.0f, 0.7f, 0.1f, 1.0f}; // 色

	// == メニュー ==
	Vector2 startPos_ = {520.0f, 400.0f};            // 座標
	float offsetY_ = 90.0f;                          // オフセット
	Vector4 normalColor_ = {1.0f, 1.0f, 1.0f, 1.0f}; // 非セレクト時の色
	Vector4 selectColor_ = {1.0f, 0.7f, 0.1f, 1.0f}; // セレクト時の色

	// == 背景 ==
	std::unique_ptr<TinyEngine::Sprite> background_;
	Vector3 voronoiParams_ = {5.0f, 2.0f, 1.0f};         // ボロノイノイズのパラメータ
	float voronoiTimer_ = 0.0f;                          // ボロノイノイズ用タイマー
	Vector4 voronoiColor_ = {0.0f, 0.2f, 0.2f, 1.0f};    // ボロノイノイズの色
	Vector4 backgroundColor_ = {0.0f, 0.0f, 0.0f, 1.0f}; // 背景色
};
