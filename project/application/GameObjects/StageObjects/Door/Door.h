#pragma once
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "AABB.h"
#include "GameObjects/IGameObject.h"
#include "GameObjects/StageObjects/Wall/WallStatus.h"

/// <summary>
/// ステージに配置するドア
/// </summary>
class Door : public IGameObject{
public:
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
	std::string GetName() const override { return "Door"; }

private:
	AABB collision_; // 当たり判定
	WallStatus wallStatus_;
	Vector4 color_ = {0.1f, 0.1f, 0.18f, 1.0f};
	float time_ = 0.0f;
	bool isOpen_ = false;
	Vector2 collisionSize_ = {4.0f, 4.0f};

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
};
