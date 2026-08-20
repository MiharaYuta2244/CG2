#include "ResultScoreDisplay.h"
#include <iomanip>
#include <sstream>

using namespace TinyEngine;

void ResultScoreDisplay::Initialize(EngineContext* ctx, bool isClear, float clearTime, int killCount) {

	// クリアタイム表示の初期化
	timeSprites_.clear();

	// ワイプアニメ開始タイマー
	wipeAnimStartTimer_.Initialize(1.0f);

	if (isClear) {
		// クリア時は時間を分:秒に変換して表示
		int minutes = static_cast<int>(clearTime) / 60;
		int seconds = static_cast<int>(clearTime) % 60;

		// 分が99を超えないように制限
		minutes = std::min(minutes, 99);

		// MM:SSの形式を作成
		int m10 = minutes / 10;
		int m1 = minutes % 10;
		int s10 = seconds / 10;
		int s1 = seconds % 10;

		timeSprites_.push_back(CreateDigitSprite(ctx, digitTextureNames_[m10], kTimeBasePos));
		timeSprites_.push_back(CreateDigitSprite(ctx, digitTextureNames_[m1], {kTimeBasePos.x + kDigitSpacing, kTimeBasePos.y}));

		colonSprite_ = CreateSymbolSprite(ctx, "colon.png", {kTimeBasePos.x + kDigitSpacing * 2, kTimeBasePos.y});

		timeSprites_.push_back(CreateDigitSprite(ctx, digitTextureNames_[s10], {kTimeBasePos.x + kDigitSpacing * 3, kTimeBasePos.y}));
		timeSprites_.push_back(CreateDigitSprite(ctx, digitTextureNames_[s1], {kTimeBasePos.x + kDigitSpacing * 4, kTimeBasePos.y}));
	} else {
		// ゲームオーバー時は "--:--" を表示
		timeSprites_.push_back(CreateSymbolSprite(ctx, "hyphen.png", kTimeBasePos));
		timeSprites_.push_back(CreateSymbolSprite(ctx, "hyphen.png", {kTimeBasePos.x + kDigitSpacing, kTimeBasePos.y}));

		colonSprite_ = CreateSymbolSprite(ctx, "colon.png", {kTimeBasePos.x + kDigitSpacing * 2, kTimeBasePos.y});

		timeSprites_.push_back(CreateSymbolSprite(ctx, "hyphen.png", {kTimeBasePos.x + kDigitSpacing * 3, kTimeBasePos.y}));
		timeSprites_.push_back(CreateSymbolSprite(ctx, "hyphen.png", {kTimeBasePos.x + kDigitSpacing * 4, kTimeBasePos.y}));
	}

	// キル数表示の初期化
	killCountSprites_.clear();

	if (isClear) {
		// 3桁上限に制限
		int clampedKillCount = std::min(std::max(killCount, 0), 999);

		int k100 = clampedKillCount / 100;
		int k10 = (clampedKillCount / 10) % 10;
		int k1 = clampedKillCount % 10;

		killCountSprites_.push_back(CreateDigitSprite(ctx, digitTextureNames_[k100], kKillCountBasePos));
		killCountSprites_.push_back(CreateDigitSprite(ctx, digitTextureNames_[k10], {kKillCountBasePos.x + kDigitSpacing, kKillCountBasePos.y}));
		killCountSprites_.push_back(CreateDigitSprite(ctx, digitTextureNames_[k1], {kKillCountBasePos.x + kDigitSpacing * 2, kKillCountBasePos.y}));

		unitSprite_ = CreateSymbolSprite(ctx, "tai.png", {kKillCountBasePos.x + kDigitSpacing * 3, kKillCountBasePos.y});

	} else {
		// ゲームオーバー時は "-体" を表示
		killCountSprites_.push_back(CreateSymbolSprite(ctx, "hyphen.png", {kKillCountBasePos.x + kDigitSpacing, kKillCountBasePos.y}));
		unitSprite_ = CreateSymbolSprite(ctx, "tai.png", {kKillCountBasePos.x + kDigitSpacing * 2, kKillCountBasePos.y});
	}

	// ワイプアニメーションの初期化
	Vector2 wipeSize = {350.0f, 100.0f};

	timerWipe_ = std::make_unique<SpriteScaleWipeAnimator>();
	timerWipe_->Initialize(ctx, kTimeBasePos, wipeSize);

	killCountWipe_ = std::make_unique<SpriteScaleWipeAnimator>();
	killCountWipe_->Initialize(ctx, kKillCountBasePos, wipeSize);

	// タイマー側のワイプを即座に開始
	timerWipe_->StartAnimation();
}

void ResultScoreDisplay::Update(float deltaTime) {
	// ワイプアニメ開始タイマー更新
	wipeAnimStartTimer_.Update(deltaTime);

	// ワイプの更新
	if (wipeAnimStartTimer_.IsEnd()) {
		timerWipe_->Update(deltaTime);

		// キル数ワイプを少し遅れて開始させる
		if (!isKillCountWipeStarted_) {
			killCountWipeDelay_ -= deltaTime;
			if (killCountWipeDelay_ <= 0.0f) {
				killCountWipe_->StartAnimation();
				isKillCountWipeStarted_ = true;
			}
		} else {
			killCountWipe_->Update(deltaTime);
		}
	}

	// スプライトの更新
	for (auto& sprite : timeSprites_) {
		sprite->Update();
	}
	if (colonSprite_)
		colonSprite_->Update();

	for (auto& sprite : killCountSprites_) {
		sprite->Update();
	}
	if (unitSprite_)
		unitSprite_->Update();
}

void ResultScoreDisplay::Draw() {
	// クリアタイマー描画
	if (timerWipe_->GetBeforeAnimationFinished()) {
		for (auto& sprite : timeSprites_) {
			sprite->Draw();
		}
		if (colonSprite_) {
			colonSprite_->Draw();
		}
	}

	// タイマー用ワイプ描画
	timerWipe_->Draw();

	// キル数描画
	if (killCountWipe_->GetBeforeAnimationFinished()) {
		for (auto& sprite : killCountSprites_) {
			sprite->Draw();
		}
		if (unitSprite_) {
			unitSprite_->Draw();
		}
	}

	// スコア用ワイプ描画
	killCountWipe_->Draw();
}

std::unique_ptr<Sprite> ResultScoreDisplay::CreateDigitSprite(EngineContext* ctx, const std::string& textureName, const Vector2& pos) {
	auto sprite = std::make_unique<Sprite>();
	sprite->Initialize(ctx, textureName);
	sprite->SetPosition(pos);
	return sprite;
}

std::unique_ptr<Sprite> ResultScoreDisplay::CreateSymbolSprite(EngineContext* ctx, const std::string& textureName, const Vector2& pos) {
	auto sprite = std::make_unique<Sprite>();
	sprite->Initialize(ctx, textureName);
	sprite->SetPosition(pos);
	return sprite;
}