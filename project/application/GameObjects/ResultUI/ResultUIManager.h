#pragma once 
#include "Text/ResultScoreText.h"
#include "Text/ResultTimerText.h"
#include "Button/RetryButton.h"
#include "Button/ToTitleButton.h"

/// <summary>
/// リザルトシーンのUI管理クラス
/// </summary>
class ResultUIManager {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, DirectInput* input);

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
	std::unique_ptr<ResultScoreText> scoreText_;

	// クリアタイマーテキスト
	std::unique_ptr<ResultTimerText> timerText_;

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
};
