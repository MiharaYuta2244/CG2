#pragma once
#include <nlohmann/json.hpp>

struct GlassStatus {
	float width;
	float depth;
	float centerX;
	float centerZ;
	float rotateY;
};

// セーブ用 GlassStatus -> Jsonへの変換ルール
inline void to_json(Json& j, const GlassStatus& wState) {
	j = Json{
	    {"width",   wState.width  },
        {"depth",   wState.depth  },
        {"centerX", wState.centerX},
        {"centerZ", wState.centerZ},
        {"rotateY", wState.rotateY}
    };
}

// ロード用 Json -> GlassStatusへの変換ルール
inline void from_json(const Json& j, GlassStatus& wState) {
	wState.width = j.value("width", 0.0f);
	wState.depth = j.value("depth", 0.0f);
	wState.centerX = j.value("centerX", 0.0f);
	wState.centerZ = j.value("centerZ", 0.0f);
	wState.rotateY = j.value("rotateY", 0.0f);
}