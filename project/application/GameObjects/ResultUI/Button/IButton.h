#pragma once
#include "Sprite.h"

/// <summary>
/// 選択しているボタンのタイプ
/// </summary>
struct ButtonStyle{
	Vector4 color;
	Vector2 size;
};

/// <summary>
/// シーン切り替え用ボタンの基底クラス
/// </summary>
class IButton {
public:
	// 初期化
	void Initialize(EngineContext* ctx);

	// 更新
	void Update(bool isSelect);

	// 描画
	void Draw();

	// コールバック登録
	void SetCallback(std::function<void()> callback);

	// 決定処理
	void Execute();

protected:
	virtual const ButtonStyle& GetNormalStyle() const = 0;
	virtual const ButtonStyle& GetSelectedStyle() const = 0;

protected:
	// スプライト
	std::unique_ptr<TinyEngine::Sprite> sprite_;

	// 選択フラグ
	bool isSelect_ = false;

	// 座標
	Vector2 pos_{};

private:
	std::function<void()> callback_;
};