#pragma once
#include "AABB.h"
#include "GameObjects/ObjectRender/ObjectRender.h"

struct WallStatus {
	float width;
	float depth;
	float centerX;
	float centerZ;
};

/// <summary>
/// 壁クラス
/// </summary>
class Wall {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, WallStatus wallStatus);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// 当たり判定Getter
	AABB GetCollision() const { return collision_; }

	// WallStatusのSetter
	void SetWallStatus(WallStatus wallStatus);

	// WallStatusのGetter
	WallStatus& GetWallStatus() { return wallStatus_; }

private:
	Transform transform_;
	AABB collision_; // 当たり判定
	WallStatus wallStatus_;
	Vector4 color_ = {1, 1, 1, 1};

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
};
