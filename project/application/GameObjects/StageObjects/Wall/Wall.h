#pragma once
#include "AABB.h"
#include "GameObjects/IGameObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"

struct WallStatus {
	float width;
	float depth;
	float centerX;
	float centerZ;
};

// セーブ用 WallStatus -> Jsonへの変換ルール
inline void to_json(Json& j, const WallStatus& wState) {
	j = Json{
	    {"width",   wState.width  },
        {"depth",   wState.depth  },
        {"centerX", wState.centerX},
        {"centerZ", wState.centerZ}
    };
}

// ロード用 Json -> WallStatusへの変換ルール
inline void from_json(const Json& j, WallStatus& wState) {
	wState.width = j.value("width", 0.0f);
	wState.depth = j.value("depth", 0.0f);
	wState.centerX = j.value("centerX", 0.0f);
	wState.centerZ = j.value("centerZ", 0.0f);
}

/// <summary>
/// 壁クラス
/// </summary>
class Wall : public IGameObject {
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
	std::string GetName() const override { return "Wall"; }

private:
	AABB collision_; // 当たり判定
	WallStatus wallStatus_;
	Vector4 color_ = {0.1f, 0.1f, 0.18f, 1.0f};
	float time_ = 0.0f;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
};
