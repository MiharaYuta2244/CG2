#pragma once
#include "Wall.h"
#include <list>
#include <string>

/// <summary>
/// 壁オブジェクト管理クラス
/// </summary>
class WallManager {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// ImGui
	void DrawImGui();

	// 壁のリストを取得するGetter
	const std::list<std::unique_ptr<Wall>>& GetWalls() const { return walls_; }

private:
	// Json読み込み
	void LoadFromJson(const std::string& filepath);

	// Json書き込み
	void SaveToJson(const std::string& filepath);

private:
	EngineContext* ctx_ = nullptr;
	std::string jsonPath_;

	// 壁オブジェクトのリスト
	std::list<std::unique_ptr<Wall>> walls_;
};
