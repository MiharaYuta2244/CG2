#pragma once
#include <string>
#include <vector>

enum class TileType {
	EMPTY = 0,
	BLOCK = 1,
};

class Map {
public:
	void Initialize();

	// CCSVファイルからマップチップデータの読み込み格納
	std::vector<std::vector<int>> LoadMapCSV(const std::string& filename);

	// タイルタイプ取得
	int GetMapData(int row, int col) const;

	// 行数を取得
	int GetRowCount() const { return static_cast<int>(mapData1_.size()); }

	// 列数を取得
	int GetColumnCount() const { return mapData1_.empty() ? 0 : static_cast<int>(mapData1_[0].size()); }

private:
	// マップデータ
	std::vector<std::vector<int>> mapData1_;
};
