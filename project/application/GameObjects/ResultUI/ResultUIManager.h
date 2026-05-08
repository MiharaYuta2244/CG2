#pragma once 
#include "ResultScoreText.h"
#include "ResultTimerText.h"

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

private:
	// スコアテキスト
	std::unique_ptr<ResultScoreText> scoreText_;

	// クリアタイマーテキスト
	std::unique_ptr<ResultTimerText> timerText_;
};
