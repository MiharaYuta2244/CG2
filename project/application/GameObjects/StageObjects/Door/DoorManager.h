#pragma once
#include "Door.h"
#include <list>
#include <string>

class DoorManager {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, Vector3 playerPos);

	// 描画処理
	void Draw();

	// ImGui
	void DrawImGui();

	// 壁のリストを取得するGetter
	const std::list<std::unique_ptr<Door>>& GetDoors() const { return doors_; }

private:
	// Json読み込み
	void LoadFromJson(const std::string& filepath);

	// Json書き込み
	void SaveToJson(const std::string& filepath);

private:
	EngineContext* ctx_ = nullptr;
	std::string jsonPath_;

	// 壁オブジェクトのリスト
	std::list<std::unique_ptr<Door>> doors_;
};
