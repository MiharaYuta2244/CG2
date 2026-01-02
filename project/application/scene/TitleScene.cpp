#include "TitleScene.h"
#include "Easing.h"
#include "SceneManager.h"
#include <algorithm>
#include <numbers>

void TitleScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	// タイトルテキストモデル
	titleText_ = std::make_unique<TitleText>();
	titleText_->Initialize(engineContext_);

	// はじめるモデル
	startModel_ = std::make_unique<StartModel>();
	startModel_->Initialize(engineContext_);

	// おわるモデル
	endModel_ = std::make_unique<EndModel>();
	endModel_->Initialize(engineContext_);

	// 画面両端の幕
	rightCurtain_ = std::make_unique<BothCurtain>();
	rightCurtain_->Initialize(ctx);
	leftCurtain_ = std::make_unique<BothCurtain>();
	leftCurtain_->Initialize(ctx);

	// menuLayoutsの初期化
	InitMenuLayouts();

	// タイトルメニューモデル
	for (size_t i = 0; i < titleMenuModels_.size(); i++) {
		titleMenuModels_[i] = std::make_unique<TitleMenuModel>();
		titleMenuModels_[i]->Initialize(engineContext_, positions_[i]);
	}

	// 読み込むモデル
	titleMenuModels_[0]->SetModel("return.obj");
	titleMenuModels_[1]->SetModel("stage1.obj");
	titleMenuModels_[2]->SetModel("stage1.obj");
	titleMenuModels_[3]->SetModel("stage1.obj");
	titleMenuModels_[4]->SetModel("charaSelect.obj");

	titleMenuModels_[2]->SetColor({1.0f, 0.0f, 0.0f, 1.0f});
	titleMenuModels_[3]->SetColor({0.0f, 0.0f, 1.0f, 1.0f});

	// タイトルの状態
	titleState_ = TitleState::START;

	// タイトルシーン1か2か
	titleNumber_ = TitleNumber::TITLE1;

	// スプライトの生成&初期化
	CreateAndInitializeSprites();
}

void TitleScene::Update() {
	// モデルの更新　タイトル1
	Title1Update();

	// モデルの更新　タイトル2
	Title2Update();

	// 両シーン共通の更新処理
	rightCurtain_->Update();
	leftCurtain_->Update();

	// タイトルの状態切り替え
	StateChange();

	// シーン切り替え処理
	ChangeScene();
}

void TitleScene::Draw() {
	// タイトル番号が1の時
	Title1Draw();

	// タイトル番号が2の時
	Title2Draw();

	// 両シーン共通の描画処理
	rightCurtain_->Draw();
	leftCurtain_->Draw();
}

void TitleScene::Finalize() {
	titleText_.reset();
	startModel_.reset();
	endModel_.reset();
}

void TitleScene::ChangeScene() {
	if (titleNumber_ == TitleNumber::TITLE2)
		return;

	// 入力
	bool upInput = keyboard_->KeyTriggered(DIK_W) || gamePad_->GetState().buttonsPressed.dpadUp;
	bool downInput = keyboard_->KeyTriggered(DIK_S) || gamePad_->GetState().buttonsPressed.dpadDown;

	// 上入力の場合
	if (upInput) {
		titleState_ = TitleState::START; // ゲーム開始
		startModel_->SetSelected(true);  // 選択状態にする
		endModel_->SetSelected(false);   // 選択状態を解除
	}

	// 下入力の場合
	if (downInput) {
		titleState_ = TitleState::END;   // ゲーム終了
		endModel_->SetSelected(true);    // 選択状態にする
		startModel_->SetSelected(false); // 選択状態を解除
	}

	// ゲーム開始入力
	bool startInput = keyboard_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;
	if (startInput && titleState_ == TitleState::START) {
		titleNumber_ = TitleNumber::TITLE2;   // タイトル2へ
		titleState_ = TitleState::BACK_SCENE; // タイトルの状態をバックシーンに変更
	} else if (startInput && titleState_ == TitleState::END) {
		PostQuitMessage(0); // ゲーム終了
	}
}

void TitleScene::StateChange() {
	if (titleNumber_ == TitleNumber::TITLE1)
		return;

	// 入力
	bool upInput = keyboard_->KeyTriggered(DIK_W) || gamePad_->GetState().buttonsPressed.dpadUp;
	bool downInput = keyboard_->KeyTriggered(DIK_S) || gamePad_->GetState().buttonsPressed.dpadDown;
	bool rightInput = keyboard_->KeyTriggered(DIK_D) || gamePad_->GetState().buttonsPressed.dpadRight;
	bool leftInput = keyboard_->KeyTriggered(DIK_A) || gamePad_->GetState().buttonsPressed.dpadLeft;
	bool decideInput = keyboard_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;

	// 状態遷移テーブル
	static const std::unordered_map<TitleState, StateTransition> transitions = {
	    {TitleState::BACK_SCENE,
	     {TitleState::CHARACTER_SELECT,           // left
	      TitleState::STAGE1,                     // right
	      [this]() {                              // decide
		      keyboard_->Reset();                 // 入力状態をリセット
		      titleNumber_ = TitleNumber::TITLE1; // タイトル番号を元に戻す
		      titleState_ = TitleState::START;    // タイトルの状態も初期状態元に戻す
	      }}	                                                                                                                               },
	    {TitleState::STAGE1,           {TitleState::BACK_SCENE, TitleState::STAGE2, [this]() { sceneManager_->ChangeScene("GamePlay"); }}      }, // ステージ1へ
	    {TitleState::STAGE2,           {TitleState::STAGE1, TitleState::STAGE3, [this]() { sceneManager_->ChangeScene("GamePlay"); }}          }, // ステージ2へ
	    {TitleState::STAGE3,           {TitleState::STAGE2, TitleState::CHARACTER_SELECT, [this]() { sceneManager_->ChangeScene("GamePlay"); }}}, // ステージ3へ
	    {TitleState::CHARACTER_SELECT, {TitleState::STAGE3, TitleState::BACK_SCENE, [this]() { isCharacterSelection_ = true; }}                }, // キャラクターセレクトへ
	};

	const auto& t = transitions.at(titleState_);

	if (leftInput)
		titleState_ = t.left;
	if (rightInput)
		titleState_ = t.right;
	if (decideInput)
		t.onDecide();

	// 選択状態に応じてモデルの座標変更
	if (prevState_ != titleState_) {
		ApplyMenuLayout();
	}

	// タイトルの状態を記録
	prevState_ = titleState_;
}

void TitleScene::Title1Update() {
	if (titleNumber_ == TitleNumber::TITLE1) {
		titleText_->Update(timeManager_->GetDeltaTime());
		startModel_->Update(timeManager_->GetDeltaTime());
		endModel_->Update(timeManager_->GetDeltaTime());
	}
}

void TitleScene::Title2Update() {
	if (titleNumber_ == TitleNumber::TITLE2) {
		for (auto& model : titleMenuModels_) {
			model->Update(timeManager_->GetDeltaTime());
		}

		selectSprite_->Update();
		selectIconSprite_->Update();
		menuSprite_->Update();
	}
}

void TitleScene::Title1Draw() {
	if (titleNumber_ == TitleNumber::TITLE1) {
		titleText_->Draw();
		startModel_->Draw();
		endModel_->Draw();
	}
}

void TitleScene::Title2Draw() {
	if (titleNumber_ == TitleNumber::TITLE2) {
		for (auto& model : titleMenuModels_) {
			model->Draw();
		}
	}

	selectSprite_->Draw();
	selectIconSprite_->Draw();
	menuSprite_->Draw();
}

void TitleScene::ApplyMenuLayout() {
	const auto& layout = menuLayouts.at(titleState_);

	for (size_t i = 0; i < titleMenuModels_.size(); i++) {
		titleMenuModels_[i]->SetTranslate(layout.positions[i]);
	}
}

void TitleScene::InitMenuLayouts() {
	for (auto& [state, shift] : shiftTable) {

		MenuLayout layout{};

		for (int i = 0; i < positions_.size(); i++) {
			layout.positions[i] = positions_[(i - shift + positions_.size()) % positions_.size()];
		}

		menuLayouts[state] = layout;
	}
}

void TitleScene::CreateAndInitializeSprites() {
	selectSprite_ = std::make_unique<Sprite>();
	selectSprite_->Initialize(engineContext_, "resources/select.png");
	selectSprite_->GetPosition() = {240.0f, 620.0f};
	selectSprite_->SetSize({124.0f, 32.0f});

	selectIconSprite_ = std::make_unique<Sprite>();
	selectIconSprite_->Initialize(engineContext_, "resources/selectIcon.png");
	selectIconSprite_->GetPosition() = {200.0f, 620.0f};
	selectIconSprite_->SetSize({32.0f, 32.0f});

	menuSprite_ = std::make_unique<Sprite>();
	menuSprite_->Initialize(engineContext_, "resources/menu.png");
	menuSprite_->GetPosition() = {200.0f, 50.0f};
	menuSprite_->SetSize({256.0f, 64.0f});
}
