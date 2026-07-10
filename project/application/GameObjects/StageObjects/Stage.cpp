#include "Stage.h"

void Stage::Initialize(const SceneContext& ctx) {
	// 壁の管理インスタンス生成&初期化
	wallManager_ = std::make_unique<WallManager>();
	wallManager_->Initialize(ctx.engineContext);

	// ドアの管理インスタンス生成&初期化
	doorManager_ = std::make_unique<DoorManager>();
	doorManager_->Initialize(ctx.engineContext);

	// ガラスの管理インスタンス生成&初期化
	glassManager_ = std::make_unique<GlassManager>();
	glassManager_->Initialize(ctx.engineContext);

	// 檻の管理インスタンス生成&初期化
	cageManager_ = std::make_unique<CageManager>();
	cageManager_->Initialize(ctx.engineContext);

	// 回復エリアの管理インスタンス生成&初期化
	healAreaManager_ = std::make_unique<HealAreaManager>();
	healAreaManager_->Initialize(ctx.engineContext);

	// 地面の生成&初期化
	ground_ = std::make_unique<Ground>();
	ground_->Initialize(ctx.engineContext);

	// ゴール判定インスタンス生成&初期化
	goal_ = std::make_unique<Goal>();
	goal_->Initialize(ctx.engineContext);
}

void Stage::Update(float deltaTime, const Vector3& playerPos) {
	wallManager_->Update(deltaTime);
	doorManager_->Update(deltaTime, playerPos);
	glassManager_->Update();
	cageManager_->Update(deltaTime);
	healAreaManager_->Update(deltaTime);
	ground_->Update();
	goal_->Update(deltaTime);
}

void Stage::Draw() {
	ground_->Draw();
	doorManager_->Draw();
	wallManager_->Draw();
	cageManager_->Draw();
	goal_->Draw();
}

void Stage::DrawTransparent() { 
	glassManager_->Draw();
	healAreaManager_->Draw();
}

void Stage::DrawImGui() {
	wallManager_->DrawImGui();
	doorManager_->DrawImGui();
	glassManager_->DrawImGui();
	cageManager_->DrawImGui();
	healAreaManager_->DrawImGui();
}