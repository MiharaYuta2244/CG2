#pragma once
#include "AABB.h"
#include "GameObjects/IGameObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "GlassStatus.h"

class Glass : public IGameObject {
public:
	void Initialize(EngineContext* ctx, GlassStatus glassStatus);

	void Update();

	void Draw();

	// 当たり判定Getter
	AABB GetCollision() const { return collision_; }

	// GlassStatusのGetter
	GlassStatus& GetGlassStatus() { return glassStatus_; }

	// GlassStatusのSetter
	void SetGlassStatus(GlassStatus glassStatus);

private:
	// 当たり判定の更新
	void UpdateCollision();

private:
	// 描画用インスタンス
	std::unique_ptr<ObjectRender> render_;

	// 当たり判定
	AABB collision_;

	// ガラスのステータス
	GlassStatus glassStatus_;
};
