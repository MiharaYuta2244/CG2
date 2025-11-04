#include <fstream>
#include <sstream>
#include "Map.h"

void Map::Initialize() {
	// マップチップデータ読み込み格納
	mapData1_ = LoadMapCSV("resources/map/map1.csv");
}

std::vector<std::vector<int>> Map::LoadMapCSV(const std::string& filename) {
	std::vector<std::vector<int>> mapData;
	std::ifstream file(filename);
	std::string line;

	while (std::getline(file, line)) {
		std::vector<int> row;
		std::stringstream ss(line);
		std::string cell;

		while (std::getline(ss, cell, ',')) {
			row.push_back(std::stoi(cell));
		}

		mapData.push_back(row);
	}

	return mapData;
}

int Map::GetMapData(int row, int col) const {
	if (row < 0 || row >= GetRowCount() || col < 0 || col >= GetColumnCount()) {
		return static_cast<int>(TileType::EMPTY); // 範囲外は空タイル
	}
	return mapData1_[row][col];
}