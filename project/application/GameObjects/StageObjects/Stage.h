#pragma once
#include "Cage/CageManager.h"
#include "Door/DoorManager.h"
#include "Glass/GlassManager.h"
#include "Goal/Goal.h"
#include "Ground/Ground.h"
#include "SceneContext.h"
#include "Wall/WallManager.h"
#include "HealArea/HealAreaManager.h"
#include <memory>

/// <summary>
/// ステージ構成要素を統括するクラス
/// </summary>
class Stage {
public:
	// 初期化
	void Initialize(const SceneContext& ctx, TinyEngine::DecalManager* decalManager);

	// 更新
	void Update(float deltaTime, const Vector3& playerPos, Camera* camera);

	// 不透明オブジェクトの描画
	void Draw();

	// 半透明オブジェクトの描画
	void DrawTransparent();

	// ImGuiの描画
	void DrawImGui();

	// 各種マネージャーへのアクセス用ゲッター
	WallManager* GetWallManager() const { return wallManager_.get(); }
	DoorManager* GetDoorManager() const { return doorManager_.get(); }
	GlassManager* GetGlassManager() const { return glassManager_.get(); }
	CageManager* GetCageManager() const { return cageManager_.get(); }
	HealAreaManager* GetHealAreaManager() const { return healAreaManager_.get(); }
	Goal* GetGoal() const { return goal_.get(); }
	Ground* GetGround() const { return ground_.get(); }

private:
	std::unique_ptr<WallManager> wallManager_;
	std::unique_ptr<DoorManager> doorManager_;
	std::unique_ptr<GlassManager> glassManager_;
	std::unique_ptr<CageManager> cageManager_;
	std::unique_ptr<HealAreaManager> healAreaManager_;
	std::unique_ptr<Ground> ground_;
	std::unique_ptr<Goal> goal_;
	TinyEngine::DecalManager* decalManager_;
};