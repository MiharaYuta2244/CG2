#pragma once
#include "Transform.h"
#include <vector>
#include <string>

struct ObjectData {
	std::string type;
	std::string name;
	Transform transform;
	std::vector<ObjectData> children;
};

struct LevelData {
	std::string name;
	std::vector<ObjectData> objects;
};

/// <summary>
/// 外部ファイルからオブジェクトの配置データ等を読み込むクラス
/// </summary>
class LevelDataLoader {
public:
	static bool LoadLevel(const std::string& filepath, LevelData& outLevel);
};
