#pragma once
#include "BaseScene.h"
#include "Menu/MenuList.h"
#include "Sprite.h"

class StageSelectScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// メニュー（ステージ一覧）
	std::unique_ptr<MenuList> menu_;

	// 背景
	std::unique_ptr<TinyEngine::Sprite> background_;

	// シーン間共有データ
	CommonData* commonData_ = nullptr;

	// ステージセレクトテキスト
	std::array<std::unique_ptr<TinyEngine::Sprite>, 2> stageSelectText_;
	Vector3 stageSelectUvTranslate_{};
	float uvMoveSpeed_ = 0.2f;

	// ポストエフェクトパラメータ
	ScanlineParam scanlineParam_;                 // 走査線
	BarrelDistortionParam barrelDistortionParam_; // 魚眼
	GlitchParam glitchParam_;                     // グリッチ
};