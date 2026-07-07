#pragma once
#include <nlohmann/json.hpp>

struct CageStatus {
	float width;
	float depth;
	float centerX;
	float centerZ;
	float rotateY;
};

// セーブ用 CageStatus -> Jsonへの変換ルール
inline void to_json(Json& j, const CageStatus& cState) {
	j = Json{
	    {"width",   cState.width  },
        {"depth",   cState.depth  },
        {"centerX", cState.centerX},
        {"centerZ", cState.centerZ},
        {"rotateY", cState.rotateY}
    };
}

// ロード用 Json -> CageStatusへの変換ルール
inline void from_json(const Json& j, CageStatus& cState) {
	cState.width = j.value("width", 0.0f);
	cState.depth = j.value("depth", 0.0f);
	cState.centerX = j.value("centerX", 0.0f);
	cState.centerZ = j.value("centerZ", 0.0f);
	cState.rotateY = j.value("rotateY", 0.0f);
}