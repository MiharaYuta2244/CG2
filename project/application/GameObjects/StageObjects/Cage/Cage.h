#pragma once
#include "AABB.h"
#include "GameObjects/IGameObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "CageStatus.h"

/// <summary>
/// 檻クラス
/// </summary>
class Cage : public IGameObject {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, CageStatus cageStatus);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// 当たり判定Getter
	AABB GetCollision() const { return collision_; }

	// CageStatusのSetter
	void SetCageStatus(CageStatus cageStatus);

	// CageStatusのGetter
	CageStatus& GetCageStatus() { return cageStatus_; }

	// ギズモ用
	std::string GetName() const override { return "Cage"; }

private:
	AABB collision_; // 当たり判定
	CageStatus cageStatus_;
	Vector4 color_ = {0.1f, 0.1f, 0.1f, 1.0f};
	float time_ = 0.0f;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
};
