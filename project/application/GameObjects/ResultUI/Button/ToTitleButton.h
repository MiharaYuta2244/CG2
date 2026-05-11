#pragma once
#include "IButton.h"

/// <summary>
/// タイトルへ戻るボタン
/// </summary>
class ToTitleButton : public IButton{
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(bool isSelect);

	// 描画処理
	void Draw();

private:
	const ButtonStyle& GetNormalStyle() const override;
	const ButtonStyle& GetSelectedStyle() const override;
};
