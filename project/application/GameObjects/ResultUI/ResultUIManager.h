#pragma once
#include "Button/RetryButton.h"
#include "Button/ToTitleButton.h"
#include "GameObjects/CommonData.h"
#include "Text/ResultParamText.h"

/// <summary>
/// リザルトシーンのUI管理クラス
/// </summary>
class ResultUIManager {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, CommonData* commonData);

	// 更新処理
	void Update(float deltaTime, DirectInput* input, GamePad* gamePad);

	// 描画処理
	void Draw();

	// ボタンの登録
	void RegisterButtons();

	// リトライボタンのGetter
	RetryButton* GetRetryButton() const { return retryButton_.get(); }

	// タイトルへ戻るボタンのGetter
	ToTitleButton* GetToTitleButton() const { return toTitleButton_.get(); }

private:
	// 決定ボタンを押したときの処理
	void OnDecide();

private:
	// スコアテキスト
	std::unique_ptr<ResultParamText> scoreText_;

	// クリアタイマーテキスト
	std::unique_ptr<ResultParamText> timerText_;

	// リトライボタン
	std::unique_ptr<RetryButton> retryButton_;

	// タイトルへ戻るボタン
	std::unique_ptr<ToTitleButton> toTitleButton_;

	// ボタンの選択状況
	bool isSelect_ = false;

	// ボタンのリスト
	std::vector<IButton*> buttons_;

	// 選択されているインデックス
	int selectedIndex_ = 0;

	// スティックでの連続切り換え制御用変数
	float stickCooldown_ = 0.0f;
	bool stickInUse_ = false;
};
