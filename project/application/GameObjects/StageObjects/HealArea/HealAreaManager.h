#pragma once
#include "HealArea.h"
#include <list>
#include <string>

/// <summary>
/// 回復エリア管理クラス
/// </summary>
class HealAreaManager {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// ImGui
	void DrawImGui();

	// 回復エリアのリストを取得するGetter
	const std::list<std::unique_ptr<HealArea>>& GetHealAreas() const { return healAreas_; }

private:
	// Json読み込み
	void LoadFromJson(const std::string& filepath);

	// Json書き込み
	void SaveToJson(const std::string& filepath);

private:
	EngineContext* ctx_ = nullptr;
	std::string jsonPath_;

	// 回復エリアのリスト
	std::list<std::unique_ptr<HealArea>> healAreas_;
};
