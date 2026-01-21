#include "TitleScene.h"
#include "Easing.h"
#include "MathOperator.h"
#include "MathUtility.h"
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

	audio_ = std::make_unique<XAudio>();
	audio_->Initialize();
	audio_->SoundsAllLoad("resources/TitleScene.mp3");
	audio_->SoundPlayWave();

	debugCamera_->SetTranslation({19.45f, 28.0f, -75.0f});
	debugCamera_->SetRotate({0.0f, 0.0f, 0.2f});

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

	// キャラクターセレクト画面かどうか
	isCharacterSelection_ = false;

	// モデル座標格納用配列
	positions_ = {
	    Vector3{20.0f, 2.0f, 0.0f },
        Vector3{40.0f, 2.0f, 20.0f},
        Vector3{30.0f, 2.0f, 60.0f},
        Vector3{10.0f, 2.0f, 60.0f},
        Vector3{0.0f,  2.0f, 20.0f},
	};

	shiftTable = {
	    {TitleState::BACK_SCENE,       0},
        {TitleState::STAGE1,           1},
        {TitleState::STAGE2,           2},
        {TitleState::STAGE3,           3},
        {TitleState::CHARACTER_SELECT, 4},
	};

	// タイトルメニューモデル
	for (size_t i = 0; i < titleMenuModels_.size(); i++) {
		titleMenuModels_[i] = std::make_unique<TitleMenuModel>();
		titleMenuModels_[i]->Initialize(engineContext_, positions_[i]);
	}

	// 読み込むモデル
	titleMenuModels_[0]->SetModel("return.obj");
	titleMenuModels_[1]->SetModel("stage1.obj");
	titleMenuModels_[2]->SetModel("stage2.obj");
	titleMenuModels_[3]->SetModel("stage3.obj");
	titleMenuModels_[4]->SetModel("charaSelect.obj");

	// ひとつだけ選択状態にする
	titleMenuModels_[0]->SetSelected(true);

	// タイトルの状態
	titleState_ = TitleState::START;

	// タイトルシーン1か2か
	titleNumber_ = TitleNumber::TITLE1;

	// スプライトの生成&初期化
	CreateAndInitializeSprites();

	// イージング初期化
	for (auto& easing : easingMoves_) {
		easing.isMoving = false;
	}

	// イージング回転初期化
	for (auto& easing : easingMoveRotates_) {
		easing.isActive = false;
	}

	menuSelected_ = false;

	// タイトルアニメーション
	animationTitle_.start = titleText_->GetTranslate();
	animationTitle_.end = {0.0f, 0.0f, 0.0f};
	animationTitle_.temp = animationTitle_.start;
}

void TitleScene::Update() {
	for (int i = 0; i < easingMoveRotates_.size(); i++) {
		if (!easingMoveRotates_[i].isActive) {
			for (auto& model : titleMenuModels_) {
				model->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
			}
		}
	}

	// 両シーン共通の更新処理
	rightCurtain_->Update();
	leftCurtain_->Update();

	// タイトルの状態切り替え
	StateChange();

	// モデルの更新　タイトル1
	Title1Update();

	// モデルの更新　タイトル2
	Title2Update();

	// イージング更新
	UpdateEasing();

	for (size_t i = 0; i < titleMenuModels_.size(); i++) {
		// 位置と回転のイージング更新
		Transform& transform = titleMenuModels_[i]->GetTransform();
		UpdateMoveRotateY(easingMoveRotates_[i], transform, timeManager_->GetDeltaTime());

		// 項目スプライトの変更
		ChangeMenuSelectSprite();
	}

	// シーン切り替え処理
	ChangeScene();

	// タイトル番号の記録
	prevTitleNumber_ = titleNumber_;

#ifdef USE_IMGUI
	for (auto& model : titleMenuModels_) {
		ImGui::Begin("Model");

		ImGui::DragFloat3("direction", &model->GetDirectionalLight().direction.x, 0.01f);
		ImGui::DragFloat("intensity", &model->GetDirectionalLight().intensity, 0.01f);
		ImGui::DragFloat("shininess", &model->GetMaterial().shininess, 0.01f);

		ImGui::End();
	}

	ImGui::Begin("Title");
	ImGui::DragFloat3("position", &titleText_->GetTranslate().x, 0.01f);
	ImGui::DragFloat3("rotate", &titleText_->GetRotate().x, 0.01f);
	ImGui::DragFloat3("scale", &titleText_->GetScale().x, 0.01f);
	ImGui::DragFloat3("direction", &titleText_->GetDirectLight().direction.x, 0.01f);
	ImGui::DragFloat("intensity", &titleText_->GetDirectLight().intensity, 0.01f);
	ImGui::DragFloat("shininess", &titleText_->GetMaterial().shininess, 0.01f);
	ImGui::ColorEdit4("color", &titleText_->GetColor().x);
	ImGui::End();

	ImGui::Begin("Camera");
	ImGui::DragFloat3("position", &debugCamera_->GetTranslation().x, 0.01f);
	ImGui::DragFloat3("rotate", &debugCamera_->GetRotate().x, 0.01f);
	ImGui::End();

	ImGui::Begin("StartModel");
	ImGui::DragFloat3("position", &startModel_->GetTransform().translate.x, 0.01f);
	ImGui::DragFloat3("rotate", &startModel_->GetTransform().rotate.x, 0.01f);
	ImGui::DragFloat3("scale", &startModel_->GetTransform().scale.x, 0.01f);
	ImGui::DragFloat3("direction", &startModel_->GetDirectLight().direction.x, 0.01f);
	ImGui::DragFloat("intensity", &startModel_->GetDirectLight().intensity, 0.01f);
	ImGui::DragFloat("shininess", &startModel_->GetMaterial().shininess, 0.01f);
	ImGui::ColorEdit4("color", &startModel_->GetColor().x);
	ImGui::End();

	ImGui::Begin("EndModel");
	ImGui::DragFloat3("position", &endModel_->GetTransform().translate.x, 0.01f);
	ImGui::DragFloat3("rotate", &endModel_->GetTransform().rotate.x, 0.01f);
	ImGui::DragFloat3("scale", &endModel_->GetTransform().scale.x, 0.01f);
	ImGui::DragFloat3("direction", &endModel_->GetDirectLight().direction.x, 0.01f);
	ImGui::DragFloat("intensity", &endModel_->GetDirectLight().intensity, 0.01f);
	ImGui::DragFloat("shininess", &endModel_->GetMaterial().shininess, 0.01f);
	ImGui::ColorEdit4("color", &endModel_->GetColor().x);
	ImGui::End();
#endif
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
	audio_->~XAudio();
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
		titleNumber_ = TitleNumber::TITLE2; // タイトル2へ
		titleState_ = TitleState::STAGE1;   // タイトルの状態をステージ1に変更
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
	     {TitleState::CHARACTER_SELECT, // left
	      TitleState::STAGE1,           // right
	      [this]() {                    // decide
		      OnMenuDecide();
		      menuSelected_ = true;
	      }}},
	    {TitleState::STAGE1,
	     {TitleState::BACK_SCENE, TitleState::STAGE2,
	      [this]() {
		      OnMenuDecide();
		      menuSelected_ = true;
	      }}}, // ステージ1へ
	    {TitleState::STAGE2,
	     {TitleState::STAGE1, TitleState::STAGE3,
	      [this]() {
		      OnMenuDecide();
		      menuSelected_ = true;
	      }}}, // ステージ2へ
	    {TitleState::STAGE3,
	     {TitleState::STAGE2, TitleState::CHARACTER_SELECT,
	      [this]() {
		      OnMenuDecide();
		      menuSelected_ = true;
	      }}}, // ステージ3へ
	    {TitleState::CHARACTER_SELECT,
	     {TitleState::STAGE3, TitleState::BACK_SCENE,
	      [this]() {
		      OnMenuDecide();
		      menuSelected_ = true;
	      }}}, // キャラクターセレクトへ
	};

	if (!menuSelected_) {
		const auto& t = transitions.at(titleState_);

		if (leftInput)
			titleState_ = t.left;
		if (rightInput)
			titleState_ = t.right;
		if (decideInput)
			t.onDecide();

		// 選択状態に応じてモデルの座標変更
		if (prevState_ != titleState_ && titleNumber_ == TitleNumber::TITLE2) {
			ApplyMenuLayout();
		}
	}

	// タイトルの状態を記録
	prevState_ = titleState_;
}

void TitleScene::OnMenuDecide() {
	// 選択されたモデルのインデックスを取得
	int selectedIndex = shiftTable.at(titleState_);

	// 画面中央の目標座標
	Vector3 centerPos = {20.0f, 2.0f, 30.0f};

	// 画面外の目標座標（左右で分け、円形に配置）
	std::array<Vector3, 5> offscreenPositions = {
	    Vector3{-50.0f, 2.0f,  30.0f }, // 左奥
	    Vector3{90.0f,  2.0f,  30.0f }, // 右奥
	    Vector3{20.0f,  40.0f, 0.0f  }, // 上
	    Vector3{20.0f,  2.0f,  -50.0f}, // 手前
	    Vector3{50.0f,  2.0f,  -30.0f}, // 手前右
	};

	// 各モデルのイージング移動と回転を開始
	for (size_t i = 0; i < titleMenuModels_.size(); i++) {
		// ターゲット座標を決定
		Vector3 targetPos = (static_cast<int>(i) == selectedIndex) ? centerPos : offscreenPositions[i];

		// 現在のトランスフォームを取得
		Transform& transform = titleMenuModels_[i]->GetTransform();

		// イージング回転と移動を開始
		StartMoveRotateY(easingMoveRotates_[i], transform, targetPos, 1.0f, 2);
	}
}

void TitleScene::ChangeMenuSelectSprite() {
	switch (titleState_) {
	case TitleState::STAGE1:
		menuSelectSprite_->SetTexture("resources/stage1.png");
		menuSelectSprite_->SetSize({288.0f, 64.0f});
		menuSelectSprite_->SetTextureSize({288.0f, 64.0f});
		break;
	case TitleState::STAGE2:
		menuSelectSprite_->SetTexture("resources/stage2.png");
		menuSelectSprite_->SetSize({288.0f, 64.0f});
		menuSelectSprite_->SetTextureSize({288.0f, 64.0f});
		break;
	case TitleState::STAGE3:
		menuSelectSprite_->SetTexture("resources/stage3.png");
		menuSelectSprite_->SetSize({288.0f, 64.0f});
		menuSelectSprite_->SetTextureSize({288.0f, 64.0f});
		break;
	case TitleState::BACK_SCENE:
		menuSelectSprite_->SetTexture("resources/back.png");
		menuSelectSprite_->SetSize({184.0f, 64.0f});
		menuSelectSprite_->SetTextureSize({184.0f, 64.0f});
		break;
	case TitleState::CHARACTER_SELECT:
		menuSelectSprite_->SetTexture("resources/player.png");
		menuSelectSprite_->SetSize({320.0f, 64.0f});
		menuSelectSprite_->SetTextureSize({320.0f, 64.0f});
		break;
	}
}

void TitleScene::AnimationTitle() {
	if (!animationTitle_.anim.GetIsActive() && prevTitleNumber_ != titleNumber_) {
		animationTitle_.anim = {
		    animationTitle_.start, {1.0f, 1.0f, 1.0f},
             0.5f, EaseType::EASEOUTCUBIC
        };
	}

	animationTitle_.anim.Update(timeManager_->GetDeltaTime(), animationTitle_.temp);
	titleText_->SetScale(animationTitle_.temp);
}

void TitleScene::Title1Update() {
	if (titleNumber_ == TitleNumber::TITLE1) {
		AnimationTitle();
		titleText_->Update(timeManager_->GetDeltaTime());
		startModel_->Update(timeManager_->GetDeltaTime());
		endModel_->Update(timeManager_->GetDeltaTime());
		ruleSprite1_->Update();
		ruleSprite2_->Update();
	}
}

void TitleScene::Title2Update() {
	if (titleNumber_ == TitleNumber::TITLE2) {
		for (auto& model : titleMenuModels_) {
			// 選択状況に応じてモデルの状態を変更
			if (model->GetTranslate() == positions_[0]) {
				model->SetSelected(true);
			} else {
				model->SetSelected(false);
			}

			model->Update(timeManager_->GetDeltaTime());
		}

		menuBackSprite_->Update();
		selectBackSprite_->Update();
		selectSprite_->Update();
		selectIconSprite_->Update();
		menuSprite_->Update();
		menuSelectSprite_->Update();
	}
}

void TitleScene::Title1Draw() {
	if (titleNumber_ == TitleNumber::TITLE1) {
		titleText_->Draw();
		startModel_->Draw();
		endModel_->Draw();
		ruleSprite1_->Draw();
		ruleSprite2_->Draw();
	}
}

void TitleScene::Title2Draw() {
	if (titleNumber_ == TitleNumber::TITLE2) {
		for (auto& model : titleMenuModels_) {
			model->Draw();
		}
		menuBackSprite_->Draw();
		selectBackSprite_->Draw();
		selectSprite_->Draw();
		selectIconSprite_->Draw();
		menuSprite_->Draw();
		menuSelectSprite_->Draw();
	}
}

void TitleScene::ApplyMenuLayout() {
	const auto& layout = menuLayouts.at(titleState_);

	for (size_t i = 0; i < titleMenuModels_.size(); i++) {
		// 現在の位置を開始位置に設定
		easingMoves_[i].start = titleMenuModels_[i]->GetTranslate();
		// ターゲット位置を設定
		easingMoves_[i].target = layout.positions[i];
		// イージングを開始
		easingMoves_[i].elapsed = 0.0f;
		easingMoves_[i].isMoving = true;
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

void TitleScene::UpdateEasing() {
	for (size_t i = 0; i < easingMoves_.size(); i++) {
		if (!easingMoves_[i].isMoving) {
			continue;
		}

		// 経過時間を増加
		easingMoves_[i].elapsed += timeManager_->GetDeltaTime();

		// 進行度を0～1で計算
		float progress = std::clamp(easingMoves_[i].elapsed / easingMoves_[i].duration, 0.0f, 1.0f);

		// イージング関数を適用（easeOutCubicを使用）
		float easeProgress = Easing::EaseOutCubic(progress);

		// 補間計算
		Vector3 easedPosition = MathUtility::Lerp(easingMoves_[i].start, easingMoves_[i].target, easeProgress);

		// モデルの位置を更新
		titleMenuModels_[i]->SetTranslate(easedPosition);

		// アニメーション完了時
		if (progress >= 1.0f) {
			easingMoves_[i].isMoving = false;
		}
	}
}

void TitleScene::StartMoveRotateY(EasingMoveRotate& move, const Transform& transform, const Vector3& targetPos, float duration, int rotationCount) {
	move.startPos = transform.translate;
	move.targetPos = targetPos;

	// 現在のY回転
	move.startYaw = transform.rotate.y;

	// ターゲット方向ベクトル
	Vector3 dir = targetPos - transform.translate;

	// Y軸回転
	move.targetYaw = atan2f(dir.x, dir.z);

	move.elapsed = 0.0f;
	move.duration = duration;
	move.isActive = true;
	move.rotationCount = rotationCount;
}

void TitleScene::UpdateMoveRotateY(EasingMoveRotate& move, Transform& transform, float deltaTime) {
	if (!move.isActive)
		return;

	move.elapsed += deltaTime;
	float t = move.elapsed / move.duration;
	t = std::clamp(t, 0.0f, 1.0f);

	float eased = Easing::EaseOutCubic(t);

	// 位置補間
	transform.translate = {std::lerp(move.startPos.x, move.targetPos.x, eased), std::lerp(move.startPos.y, move.targetPos.y, eased), std::lerp(move.startPos.z, move.targetPos.z, eased)};

	// Y軸回転補間（最短回転）
	float deltaYaw = move.targetYaw - move.startYaw;
	while (deltaYaw > std::numbers::pi_v<float> * 2.0f)
		deltaYaw -= std::numbers::pi_v<float> * 2.0f;
	while (deltaYaw < -std::numbers::pi_v<float> * 2.0f)
		deltaYaw += std::numbers::pi_v<float> * 2.0f;

	// 追加回転数
	float extraRotations = move.rotationCount * (std::numbers::pi_v<float> * 2.0f);

	// eased補間に追加
	transform.rotate.y = move.startYaw + (deltaYaw + extraRotations) * eased;

	// 終了処理
	if (t >= 1.0f) {
		transform.translate = move.targetPos;
		transform.rotate.y = move.targetYaw;
		move.isActive = false;

		switch (titleState_) {
		case TitleState::BACK_SCENE:
			keyboard_->Reset();                 // 入力状態をリセット
			titleNumber_ = TitleNumber::TITLE1; // タイトル番号を元に戻す
			titleState_ = TitleState::START;    // タイトルの状態も初期状態元に戻す
			menuSelected_ = false;
			break;
		case TitleState::STAGE1:
			RequestSceneChange("GamePlay"); // ステージ1へ
			break;
		case TitleState::STAGE2:
			RequestSceneChange("GamePlay"); // ステージ2へ
			break;
		case TitleState::STAGE3:
			RequestSceneChange("GamePlay"); // ステージ3へ
			break;
		case TitleState::CHARACTER_SELECT:
			RequestSceneChange("GamePlay"); // ステージ1へ
			isCharacterSelection_ = true;   // キャラクターセレクト画面へ
			break;
		}
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

	selectBackSprite_ = std::make_unique<Sprite>();
	selectBackSprite_->Initialize(engineContext_, "resources/selectBG.png");
	selectBackSprite_->GetPosition() = {160.0f, 612.0f};
	selectBackSprite_->SetSize({240, 48.0f});

	menuBackSprite_ = std::make_unique<Sprite>();
	menuBackSprite_->Initialize(engineContext_, "resources/menuBG.png");
	menuBackSprite_->GetPosition() = {160.0f, 34.0f};
	menuBackSprite_->SetSize({416.0f, 96.0f});

	ruleSprite1_ = std::make_unique<Sprite>();
	ruleSprite1_->Initialize(engineContext_, "resources/rule1.png");
	ruleSprite1_->SetPosition({200.0f, 30.0f});
	ruleSprite1_->SetSize({275.0f, 35.0f});
	ruleSprite2_ = std::make_unique<Sprite>();
	ruleSprite2_->Initialize(engineContext_, "resources/rule2.png");
	ruleSprite2_->SetPosition({200.0f, 75.0f});
	ruleSprite2_->SetSize({555.0f, 35.0f});

	// 選択中の項目スプライト
	menuSelectSprite_ = std::make_unique<Sprite>();
	menuSelectSprite_->Initialize(engineContext_, "resources/stage1.png");
	menuSelectSprite_->SetPosition({750.0f, 50.0f});
	menuSelectSprite_->SetSize({288.0f, 64.0f});
}
