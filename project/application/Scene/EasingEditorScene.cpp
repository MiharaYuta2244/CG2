#include "EasingEditorScene.h"
#include "SceneManager.h"

using namespace TinyEngine;

void EasingEditorScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;

	// イージングエディターの生成
	easingEditor_ = std::make_unique<EasingEditor>();

	// オブジェクトの生成&初期化
	object_ = std::make_unique<Object3d>();
	object_->Initialize(ctx_.engineContext, "PreviewModel");
	object_->SetModel("suzanne.obj");
}

void EasingEditorScene::Update() {
	// シーン切り替え
	if (ctx_.keyboard->KeyTriggered(DIK_SPACE)) {
		ctx_.sceneManager->ChangeScene("GamePlay");
	}

	// イージングエディターの更新処理
	easingEditor_->DrawWindow(ctx_.timeManager->GetDeltaTime());

	// アニメーション中に値をObject3dに適用
	if (easingEditor_->IsPlaying()) {
		Vector3 currentValue = easingEditor_->GetCurrentObjectValue();
		TargetType target = easingEditor_->GetTargetType();

		switch (target) {
		case TargetType::SCALE:
			object_->SetScale(currentValue);
			break;
		case TargetType::ROTATE:
			object_->SetRotate(currentValue);
			break;
		case TargetType::TRANSLATE:
			object_->SetTranslate(currentValue);
			break;
		}
	}

	// オブジェクトの更新
	object_->Update();
}

void EasingEditorScene::Draw() {
	// オブジェクトの描画
	object_->Draw();
}

void EasingEditorScene::Finalize() {

}
