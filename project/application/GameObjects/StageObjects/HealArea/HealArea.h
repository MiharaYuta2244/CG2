#pragma once
#include "GameObjects/IGameObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"

struct HealAreaStatus {
	float width;
	float depth;
	float centerX;
	float centerZ;
};

// セーブ用 HealAreaStatus -> Jsonへの変換ルール
inline void to_json(Json& j, const HealAreaStatus& wState) {
	j = Json{
	    {"width",   wState.width  },
        {"depth",   wState.depth  },
        {"centerX", wState.centerX},
        {"centerZ", wState.centerZ}
    };
}

// ロード用 Json -> HealAreaStatusへの変換ルール
inline void from_json(const Json& j, HealAreaStatus& wState) {
	wState.width = j.value("width", 0.0f);
	wState.depth = j.value("depth", 0.0f);
	wState.centerX = j.value("centerX", 0.0f);
	wState.centerZ = j.value("centerZ", 0.0f);
}

class HealArea : public IGameObject {
public:
	HealArea();

	void Initialize(EngineContext* ctx, HealAreaStatus healAreaStatus);

	void Update(float deltaTime);

	void Draw();

	// 当たり判定Getter
	AABB GetCollision() const { return collision_; }

	// HealAreaStatusのSetter
	void SetHealAreaStatus(HealAreaStatus healAreaStatus);

	// HealAreaStatusのGetter
	HealAreaStatus& GetHealAreaStatus() { return healAreaStatus_; }

	// ギズモ用
	std::string GetName() const override { return "HealArea(" + std::to_string(id_) + ")"; }

	// アクティブフラグSetter
	void SetIsActive(bool isActive) { isActive_ = isActive; }

	// アクティブフラグGetter
	bool GetIsActive() const { return isActive_; }

private:
	std::unique_ptr<ObjectRender> render_ = nullptr;
	AABB collision_{};
	HealAreaStatus healAreaStatus_{};
	bool isActive_ = true;
	Vector4 color_ = {0.0f, 1.0f, 0.0f, 0.1f};

	// オブジェクト数カウント用
	static int index;
	int id_ = 0;
};
