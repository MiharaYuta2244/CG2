#include "LevelDataLoader.h"
#include "JsonManager.h"

using json = nlohmann::json;

void from_json(const json& j, ObjectData& obj) {
	j.at("type").get_to(obj.type);
	j.at("name").get_to(obj.name);
	j.at("transform").get_to(obj.transform);

	// 子オブジェクトがあれば読み込む
	if (j.contains("children")) {
		j.at("children").get_to(obj.children);
	}
}

void from_json(const json& j, LevelData& level) {
	j.at("name").get_to(level.name);
	j.at("objects").get_to(level.objects);
}

bool LevelDataLoader::LoadLevel(const std::string& filepath, LevelData& outLevel) { return JsonManager::Load(filepath, outLevel); }
