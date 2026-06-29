#pragma once
#include <nlohmann/json.hpp>

struct DoorStatus {
	float width;
	float depth;
	float centerX;
	float centerZ;
	float rotateY;
};

// JSON 変換
inline void to_json(Json& j, const DoorStatus& s) {
	j = Json{
	    {"width",   s.width  },
        {"depth",   s.depth  },
        {"centerX", s.centerX},
        {"centerZ", s.centerZ},
        {"rotateY", s.rotateY}
    };
}

inline void from_json(const Json& j, DoorStatus& s) {
	s.width = j.value("width", 1.0f);
	s.depth = j.value("depth", 1.0f);
	s.centerX = j.value("centerX", 0.0f);
	s.centerZ = j.value("centerZ", 0.0f);
	s.rotateY = j.value("rotateY", 0.0f);
}
