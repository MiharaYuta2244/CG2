#pragma once
#include "EngineContext.h"
#include "GameObjects/ResultUI/Animation/SpriteScaleWipeAnimator.h"
#include "GameTimer.h"
#include "Sprite.h"
#include <array>
#include <memory>
#include <string>
#include <vector>

/// <summary>
/// リザルトシーンでのスコア表示クラス
/// </summary>
class ResultScoreDisplay {
public:
	// 初期化
	void Initialize(EngineContext* ctx, bool isClear, float clearTime, int killCount);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

private:
	// スプライト生成ヘルパー関数
	std::unique_ptr<TinyEngine::Sprite> CreateDigitSprite(EngineContext* ctx, const std::string& textureName, const Vector2& pos);
	std::unique_ptr<TinyEngine::Sprite> CreateSymbolSprite(EngineContext* ctx, const std::string& textureName, const Vector2& pos);

private:
	// 数字のテクスチャ名
	std::array<std::string, 10> digitTextureNames_ = {"0.png", "1.png", "2.png", "3.png", "4.png", "5.png", "6.png", "7.png", "8.png", "9.png"};

	// クリアタイム表示用
	std::vector<std::unique_ptr<TinyEngine::Sprite>> timeSprites_;
	std::unique_ptr<TinyEngine::Sprite> colonSprite_;

	// キル数表示用
	std::vector<std::unique_ptr<TinyEngine::Sprite>> killCountSprites_;
	std::unique_ptr<TinyEngine::Sprite> unitSprite_; // 「体」

	// 表示位置の基準点
	const Vector2 kTimeBasePos = {490.0f, 300.0f};
	const Vector2 kKillCountBasePos = {530.0f, 400.0f};
	const float kDigitSpacing = 60.0f; // 数字間のスペース

	// ワイプアニメーション用
	std::unique_ptr<SpriteScaleWipeAnimator> timerWipe_;
	std::unique_ptr<SpriteScaleWipeAnimator> killCountWipe_;

	// 時間差再生用タイマー
	float killCountWipeDelay_ = 0.2f; // タイマー側のワイプから少し遅れて開始する
	bool isKillCountWipeStarted_ = false;

	// ワイプアニメーション開始までのタイマー
	GameTimer wipeAnimStartTimer_;
};