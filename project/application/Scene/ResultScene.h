#pragma once
#include "BaseScene.h"
#include "GameObjects/ResultUI/ResultUIManager.h"
#include "Object3d.h"

/// <summary>
/// リザルトシーン
/// </summary>
class ResultScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// UI管理インスタンス
	std::unique_ptr<ResultUIManager> uiManager_;

	// メインカメラ
	std::unique_ptr<Camera> mainCamera_;

	// ポストエフェクトパラメータ
	ScanlineParam scanlineParam_;                 // 走査線
	BarrelDistortionParam barrelDistortionParam_; // 魚眼
	GlitchParam glitchParam_;                     // グリッチ
};
