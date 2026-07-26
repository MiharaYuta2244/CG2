#pragma once
#include "AABB.h"
#include "AnimationBundle.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "GameObjects/PlaceableObject.h"

/// <summary>
/// ステージに配置するドア
/// </summary>
class Door : public PlaceableObject {
public:
	Door();

	// 初期化処理
	void Initialize(EngineContext* ctx, const Transform& transform);

	// 更新処理
	void Update(float deltaTime, Vector3 playerPos);

	// 描画処理
	void Draw() override;

	// 当たり判定Getter
	AABB GetCollision() const { return collision_; }

	// ギズモ用
	std::string GetName() const override { return "Door(" + std::to_string(id_) + ")"; }

	// 開閉状態のGetter
	bool GetIsOpen() const { return isOpen_; }

private:
	// ドアのtransformを求める
	void CulcTransform();

	// 当たり判定の更新
	void UpdateCollision();

private:
	AABB collision_{}; // 当たり判定
	Vector4 color_ = {0.1f, 0.1f, 0.18f, 1.0f};
	float time_ = 0.0f;
	Vector2 collisionSize_ = {4.0f, 4.0f};
	Vector3 marginPos_ = {0, 0, 2};
	Transform transformSecond_{};

	// 描画用インスタンス
	std::array<std::unique_ptr<ObjectRender>, 2> renders_;

	// 移動量アニメーション
	AnimationBundle<float> openAnimBundle_;

	bool isOpen_ = false;             // 開閉状態
	bool wasOpen_ = false;            // 1フレーム前の開閉状態
	float openOffset_ = 0.0f;         // 現在のドアの開き具合
	float previousOpenOffset_ = 0.0f; // 前回フレームでの開き具合
	float maxOpenOffset_ = 2.1f;      // 最大でどれだけ開くか
	float targetDistance_ = 10.0f;    // プレイヤーがこの距離以内に近づいたら開く
	float animDuration_ = 0.8f;       // アニメーションにかかる時間

	// オブジェクト数カウント用
	static int index;
	int id_ = 0;
};
