#pragma once
#include "AABB.h"
#include "GameObjects/IGameObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "GlassStatus.h"
#include "DecalManager.h"

class Glass : public IGameObject {
public:
	Glass();

	void Initialize(EngineContext* ctx, GlassStatus glassStatus, TinyEngine::DecalManager* decalManager);

	void Update();

	void Draw();

	// 当たり判定Getter
	AABB GetCollision() const { return collision_; }

	// GlassStatusのGetter
	GlassStatus& GetGlassStatus() { return glassStatus_; }

	// GlassStatusのSetter
	void SetGlassStatus(GlassStatus glassStatus);

	// アクティブ状態のGetter
	bool GetIsActive() const { return isActive_; }

	// アクティブ状態のSetter
	void SetIsActive(bool isActive) { isActive_ = isActive; }

	// ギズモ用
	std::string GetName() const override { return "Glass(" + std::to_string(id_) + ")"; }

	// ガラスの破片を生成
	void AddGlassesDecal(Vector3 scale);

private:
	// 当たり判定の更新
	void UpdateCollision();

private:
	// 描画用インスタンス
	std::unique_ptr<ObjectRender> render_ = nullptr;

	// 当たり判定
	AABB collision_{};

	// ガラスのステータス
	GlassStatus glassStatus_{};

	// アクティブ状態
	bool isActive_ = true;

	// オブジェクト数カウント用
	static int index;
	int id_ = 0;

	// ガラスの破片の管理インスタンスポインタ
	TinyEngine::DecalManager* glassesDecalManager_ = nullptr;
};
