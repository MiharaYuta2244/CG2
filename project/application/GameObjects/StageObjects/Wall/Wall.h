#pragma once
#include "AABB.h"
#include "GameObjects/IGameObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "WallStatus.h"

/// <summary>
/// 壁クラス
/// </summary>
class Wall : public IGameObject {
public:
	Wall();

	// 初期化処理
	void Initialize(EngineContext* ctx, WallStatus wallStatus);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// 当たり判定Getter
	AABB GetCollision() const { return collision_; }

	// WallStatusのSetter
	void SetWallStatus(WallStatus wallStatus);

	// WallStatusのGetter
	WallStatus& GetWallStatus() { return wallStatus_; }

	// ギズモ用
	std::string GetName() const override { return "Wall(" + std::to_string(id_) + ")"; }

private:
	AABB collision_{}; // 当たり判定
	WallStatus wallStatus_{};
	Vector4 color_ = {0.1f, 0.1f, 0.18f, 1.0f};
	float time_ = 0.0f;

	// オブジェクト数カウント用
	static int index;
	int id_ = 0;

	// 描画用インスタンス
	std::unique_ptr<ObjectRender> render_;
};
