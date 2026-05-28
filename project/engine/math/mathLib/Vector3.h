#pragma once
#include <nlohmann/json.hpp>

using Json = nlohmann::json;

struct Vector3 final
{
	float x;
	float y;
	float z;

	bool operator==(const Vector3& other) const { return x == other.x && y == other.y && z == other.z; }
};

// セーブ用 Vector3 -> Jsonへの変換ルール
inline void to_json(Json& j, const Vector3& v) {
	j = Json{
	    {"x", v.x},
        {"y", v.y},
        {"z", v.z}
    };
}

// ロード用 Json -> Vector3への変換ルール
inline void from_json(const Json& j, Vector3& v) {
	// JSONデータが配列の場合 [x, y, z]
	if (j.is_array() && j.size() >= 3) {
		v.x = j[0].get<float>();
		v.y = j[2].get<float>();
		v.z = j[1].get<float>();
	}
}