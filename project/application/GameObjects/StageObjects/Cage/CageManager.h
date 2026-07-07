#pragma once
#include "Cage.h"
#include <list>
#include <string>

/// <summary>
/// 檻オブジェクト管理クラス
/// </summary>
class CageManager {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// ImGui
	void DrawImGui();

	// Cage のリスト取得
	const std::list<std::unique_ptr<Cage>>& GetCages() const { return cages_; }

private:
	// Json読み込み
	void LoadFromJson(const std::string& filepath);

	// Json書き込み
	void SaveToJson(const std::string& filepath);

private:
	EngineContext* ctx_ = nullptr;
	std::string jsonPath_;

	// Cageオブジェクトのリスト
	std::list<std::unique_ptr<Cage>> cages_;
};
