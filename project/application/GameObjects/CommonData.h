#pragma once

/// <summary>
/// シーンをまたいで保持するデータ
/// </summary>
struct CommonData {
	int killCount = 0;                      // 敵の死亡数
	float clearTime = 0.0f;                 // クリアタイム
	bool isClear = false;                   // クリアしたかどうか
	std::string currentStageKey = "Stage1"; // 現在プレイ中のステージ
	int currentStageNo = 1;                 // 現在のステージ番号
};
