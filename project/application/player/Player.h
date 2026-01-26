#pragma once
#include "Actor.h"
#include "AnimationBundle.h"
#include "DirectInput.h"
#include "EasingAnimation.h"
#include "EngineContext.h"
#include "HPGauge/PlayerHPGauge.h"
#include "Sprite.h"
#include "ScreenSpaceUtility.h"
#include <array>

class GamePad;

class Player : public Actor {
public:
	void Initialize(EngineContext* ctx, DirectInput* input, GamePad* gamePad);

	void Update(float deltaTime);

	void Draw();

	// Setter
	void SetVelocity(Vector2 velocity) { velocity_ = velocity; }
	void SetIsHitEnemy(bool isHpSub) { isHitEnemy_ = isHpSub; }
	void SetIsInvincible(bool isInvincible) { isInvincible_ = isInvincible; }
	void SetIsHitEnemyHipDrop(bool isHitEnemyHipDrop) { isHitEnemyHipDrop_ = isHitEnemyHipDrop; }
	void SetIsPowerUp(bool isPowerUp) { isPowerUp_ = isPowerUp; }
	void SetIsGetFruit(bool isGetFruit) { isGetFruit_ = isGetFruit; }
	void ResetHipDropDamage() { isHipDropDamage_ = false; }

	// Getter
	Vector2 GetVelocity() { return velocity_; }
	int GetIsHpSub() { return isHitEnemy_; }
	bool GetIsInvincible() { return isInvincible_; }
	bool GetIsHitEnemyHipDrop() { return isHitEnemyHipDrop_; }
	int GetHipDropPowerLevel() const { return hipDropPowerLevel_; }
	int GetHipDropDamage() const { return isHipDropDamage_; }

	// HPを取得
	int GetHP() const { return hp_; }
	bool GetIsHipDrop() const { return isRotate_; }

	// ヒップドロップパワーレベルを上昇
	void IncrementHipDropPowerLevel();

	// アニメーションの初期設定を行う
	void SetFruitGetAnimation();

private:
	enum class Direction {
		RIGHT = 0,
		LEFT = 1,
	};

private:
	// ImGui
	void UpdateImGui();

	// 横移動
	void HorizontalMove();

	// ジャンプ
	void Jump();

	// ヒップドロップ
	void HipDrop();

	// ヒップドロップアニメーション
	void AnimationHipDrop();

	// 当たり判定の位置更新
	void UpdateCollisionPos();

	// 敵との接触時の処理
	void HitEnemy();

	// 無敵状態フレームカウント
	void FrameCountIsInvincible();

	// ダメージ処理
	void SubHP();

	// ヒップドロップが敵に当たった時にプレイヤーを打ち上げる
	void AfterHipDrop();

	// スクリーン座標からワールド座標への変換
	Vector2 ScreenToWorldPoint(Vector3 worldPosition, Vector2 margin);

	// ジャンプ時スケールアニメーション
	void AnimationJump();

	// フルーツを手に入れたときのスケールアニメーション
	void AnimationFruitGet();

private:
	// 入力
	DirectInput* input_ = nullptr;

	// ジャンプ中かどうか
	bool isJump_ = false;

	// ジャンプ力
	float jumpPower_ = 20.0f;

	// ヒップドロップ力
	float hipDropPower_ = -35.0f;

	// 速度関連
	Vector2 acceleration_ = {0.0f, -25.0f};
	Vector2 velocity_ = {8.0f, 0.0f};
	float gravity_ = 0.01f;

	// ゲームパッド
	GamePad* gamePad_ = nullptr;

	// 経過時間
	float deltaTime_;

	// ヒップドロップ時の回転フラグ
	bool isRotate_ = false;

	// 地面の高さ
	float groundPosY_ = 0.0f;

	// プレイヤーの向いている方向
	Direction direction_ = Direction::RIGHT;

	// HP
	int hp_;
	const int kMaxHP = 5;

	// 敵に当たった時に立つフラグ
	bool isHitEnemy_ = false;

	// 無敵状態フラグ
	bool isInvincible_ = false;

	// 無敵時間フレームカウント
	int invincibleFrameCount_ = 0;

	// 無敵時間上限
	const int kInvincibleFrame_ = 60;

	// 敵にヒップドロップを当てた時に立つフラグ
	bool isHitEnemyHipDrop_ = false;

	// プレイヤーのパワーアップフラグ
	bool isPowerUp_ = false;

	// スプライトとプレイヤーの距離
	Vector2 spriteHPGaugeMargin_ = {-2.5f, -2.0f}; // HPゲージとプレイヤー
	Vector2 spriteLevelUpMargin_ = {0.0f, 3.0f};  // レベルアップとプレイヤー

	// コンテキスト構造体
	EngineContext* ctx_;

	// プレイヤーのヒップドロップパワー
	int hipDropPowerLevel_ = 1;

	bool isHipDropDamage_ = false;

	// ジャンプアニメーション
	AnimationBundle<Vector3> jumpScaleAnim;

	// ジャンプ後アニメーション
	AnimationBundle<Vector3> afterJumpScaleAnim;

	// プレイヤーの移動制限値
	float rightMoveLimit_ = 42.0f;
	float leftMoveLimit_ = -3.0f;

	// チャージジャンプ用のゲージ
	float chargeJumpGauge_ = 0.0f;

	// チャージジャンプの最大値
	const float kMaxChargeJumpGauge = 4.0f;

	// チャージジャンプの加算量
	float addChargeJumpAmount_ = 0.1f;

	// フルーツ取得時アニメーション
	AnimationBundle<Vector3> fruitGetAnim_;

	// フルーツ取得後アニメーション
	AnimationBundle<Vector3> afterFruitGetAnim_;

	// フルーツをゲットしたかどうか
	bool isGetFruit_ = false;

	// レベルアップスプライト
	std::unique_ptr<Sprite> levelUpSprite_;

	// レベルアップしたかどうか
	bool isLevelUp_ = false;

	// レベルアップアニメーション
	AnimationBundle<Vector2> levelUpAnimation_;

	// HPゲージ
	std::unique_ptr<PlayerHPGauge> hpGauge_;

	// 座標変換便利クラス
	std::unique_ptr<ScreenSpaceUtility> screenSpaceUtility_;
};