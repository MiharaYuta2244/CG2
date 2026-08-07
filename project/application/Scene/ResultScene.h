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
	void CreateNumberSprites(int value, const Vector2& startPos, float spacing, const SceneContext& ctx, std::vector<std::unique_ptr<TinyEngine::Sprite>>& outSprites);

private:
	// UI管理インスタンス
	std::unique_ptr<ResultUIManager> uiManager_;

	// メインカメラ
	std::unique_ptr<Camera> mainCamera_;

	// ポストエフェクトパラメータ
	ScanlineParam scanlineParam_;                 // 走査線
	BarrelDistortionParam barrelDistortionParam_; // 魚眼
	GlitchParam glitchParam_;                     // グリッチ

	std::unique_ptr<TinyEngine::Sprite> totalDamageLabel_;
	std::vector<std::unique_ptr<TinyEngine::Sprite>> totalDamageNumbers_;

	std::unique_ptr<TinyEngine::Sprite> totalDeathLabel_;
	std::vector<std::unique_ptr<TinyEngine::Sprite>> totalDeathNumbers_;
};
