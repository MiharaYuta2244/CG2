#pragma once
#include "DirectInput.h"
#include "GameObjects/Effect/DecideEffect.h"
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
	void Update(DirectInput* input, GamePad* gamePad, float deltaTime);

	// 描画処理
	void Draw();

private:
	EngineContext* ctx_ = nullptr;

	// メニューのリスト
	std::vector<MenuItem> items_;

	// 現在のインデックス
	int currentIndex_ = 0;

	// == メニュー ==
	Vector2 startPos_ = {520.0f, 400.0f};            // 座標
	float offsetY_ = 90.0f;                          // オフセット
	Vector4 normalColor_ = {1.0f, 1.0f, 1.0f, 1.0f}; // 非セレクト時の色
	Vector4 selectColor_ = {1.0f, 0.7f, 0.1f, 1.0f}; // セレクト時の色

	// 決定ボタン入力時のエフェクト
	std::unique_ptr<DecideEffect> decideEffect_;

	// スティックでの連続切り換え制御用変数
	float stickCooldown_ = 0.0f;
	bool stickInUse_ = false;
};
