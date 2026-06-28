#pragma once
#include <nlohmann/json.hpp>

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