#include "Framework.h"
#include "Game/Game.h"
#include "WinApp.h"
#include <memory>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// ゲームクラス生成
	std::unique_ptr<Framework> game = std::make_unique<Game>();

	// 実行
	game->Run();

	return 0;
}