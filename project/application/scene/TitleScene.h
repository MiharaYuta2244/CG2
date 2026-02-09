#pragma once
#include "AnimationBundle.h"
#include "BaseScene.h"
#include "BothCurtain/BothCurtain.h"
#include "EndModel/EndModel.h"
#include "Object3d.h"
#include "Sprite.h"
#include "StartModel/StartModel.h"
#include "TitleMenuModel/TitleMenuModel.h"
#include "TitleText/TitleText.h"
#include "XAudio.h"
#include <array>
#include <memory>

class TitleScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	enum class TitleState { START, END, BACK_SCENE, STAGE1, STAGE2, STAGE3, CHARACTER_SELECT };

	enum class TitleNumber { TITLE1, TITLE2 };

	struct StateTransition {
		TitleState left;
		TitleState right;
		std::function<void()> onDecide;
	};

	struct MenuLayout {
		std::array<Vector3, 5> positions;
	};

	struct EasingMove {
		Vector3 start;
		Vector3 target;
		float elapsed = 0.0f;
		float duration = 0.3f; // 移動時間（秒）
		bool isMoving = false;
	};

	struct EasingMoveRotate {
		Vector3 startPos;
		Vector3 targetPos;

		float startYaw;
		float targetYaw;

		float elapsed = 0.0f;
		float duration = 1.0f;

		bool isActive = false;
		int rotationCount;
	};

private:
	// シーン切り替え処理
	void ChangeScene();

	// タイトルの状態遷移
	void StateChange();

	// タイトル1更新処理
	void Title1Update();

	// タイトル1更新処理
	void Title2Update();

	// タイトル1の描画
	void Title1Draw();

	// タイトル2の描画
	void Title2Draw();

	// 選択状態に応じてモデルの座標変更
	void ApplyMenuLayout();

	// menuLayoutsの初期化関数
	void InitMenuLayouts();

	// スプライトの生成&初期化
	void CreateAndInitializeSprites();

	// イージング更新処理
	void UpdateEasing();

	// Y軸回転しながら移動するイージング開始
	void StartMoveRotateY(EasingMoveRotate& move, const Transform& transform, const Vector3& targetPos, float duration, int rotationCount);

	// Y軸回転しながら移動するイージング更新
	void UpdateMoveRotateY(EasingMoveRotate& move, Transform& transform, float deltaTime);

	// Space キーが押されたときの処理
	void OnMenuDecide();

	// 選択中の項目に応じてスプライトの変更
	void ChangeMenuSelectSprite();

	// タイトルテキストアニメーション
	void AnimationTitle();

private:
	// タイトルテキストモデル
	std::unique_ptr<TitleText> titleText_;

	// はじめるモデル
	std::unique_ptr<StartModel> startModel_;

	// おわるモデル
	std::unique_ptr<EndModel> endModel_;

	// ステージ1モデル
	std::array<std::unique_ptr<TitleMenuModel>, 5> titleMenuModels_;

	// タイトルの状態
	TitleState titleState_ = TitleState::START;

	// タイトルシーン1か2か
	TitleNumber titleNumber_ = TitleNumber::TITLE1;

	// せんたく画像
	std::unique_ptr<Sprite> selectSprite_;

	// せんたくアイコン画像
	std::unique_ptr<Sprite> selectIconSprite_;

	// せんたく背景画像
	std::unique_ptr<Sprite> selectBackSprite_;

	// めにゅー画像
	std::unique_ptr<Sprite> menuSprite_;

	// めにゅー背景画像
	std::unique_ptr<Sprite> menuBackSprite_;

	// 画面両端の幕
	std::unique_ptr<BothCurtain> rightCurtain_;
	std::unique_ptr<BothCurtain> leftCurtain_;

	// キャラクターセレクト画面かどうか
	bool isCharacterSelection_ = false;

	// モデル座標格納用配列
	std::array<Vector3, 5> positions_ = {
	    Vector3{20.0f, 2.0f, 0.0f },
        Vector3{40.0f, 2.0f, 20.0f},
        Vector3{30.0f, 2.0f, 60.0f},
        Vector3{10.0f, 2.0f, 60.0f},
        Vector3{0.0f,  2.0f, 20.0f},
	};

	std::unordered_map<TitleState, int> shiftTable = {
	    {TitleState::BACK_SCENE,       0},
        {TitleState::STAGE1,           1},
        {TitleState::STAGE2,           2},
        {TitleState::STAGE3,           3},
        {TitleState::CHARACTER_SELECT, 4},
	};

	// 5つのモデルの座標
	std::unordered_map<TitleState, MenuLayout> menuLayouts;

	// 1フレーム前のタイトルの状態
	TitleState prevState_;

	// イージング用データ
	std::array<EasingMove, 5> easingMoves_;

	// イージング移動回転用データ
	std::array<EasingMoveRotate, 5> easingMoveRotates_;

	// 1フレーム前のタイトル番号
	TitleNumber prevTitleNumber_;

	bool menuSelected_ = false;

	// ルール
	std::unique_ptr<Sprite> ruleSprite1_;
	std::unique_ptr<Sprite> ruleSprite2_;

	std::unique_ptr<XAudio> audio_;

	// メニューで選択中の項目スプライト
	std::unique_ptr<Sprite> menuSelectSprite_;

	// 遷移時のタイトルアニメーション
	AnimationBundle<Vector3> animationTitle_;

	// オーディオスペクトラム用スプライト
	std::vector<std::unique_ptr<Sprite>> audioSpectrumSprite_;
};
