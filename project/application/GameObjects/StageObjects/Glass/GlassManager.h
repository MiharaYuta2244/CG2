#pragma once
#include "Glass.h"
#include <list>
#include <string>

/// <summary>
/// ガラスオブジェクト管理クラス
/// </summary>
class GlassManager {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, TinyEngine::DecalManager* decalManager);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// ImGui
	void DrawImGui();

	// 壁のリストを取得するGetter
	const std::list<std::unique_ptr<Glass>>& GetGlasses() const { return glasses_; }

	// ガラスの削除処理
	void RemoveGlass(Glass* glass);

private:
	// Json読み込み
	void LoadFromJson(const std::string& filepath);

	// Json書き込み
	void SaveToJson(const std::string& filepath);

private:
	EngineContext* ctx_ = nullptr;
	std::string jsonPath_;

	// 壁オブジェクトのリスト
	std::list<std::unique_ptr<Glass>> glasses_;

	// デカール管理インスタンスポインタ
	TinyEngine::DecalManager* decalManager_ = nullptr;
};
