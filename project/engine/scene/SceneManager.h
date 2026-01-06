#pragma once
#include "BaseScene.h"
#include <functional>
#include <memory>
#include <unordered_map>

class SceneManager {
public:
	SceneManager() = default;
	~SceneManager() = default;

	// シーン初期化（共通リソースを渡す）
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager);

	// シーン追加
	void AddScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene);

	// シーン切り替え要求
	void ChangeScene(const std::string& sceneName);

	// 現在のシーンを取得
	BaseScene* GetCurrentScene() const { return currentScene_; }

	// 要求されているシーン名を取得
	std::string GetRequestedSceneName() const { return requestedSceneName_; }

	void SetRequestedSceneName(const std::string& sceneName) { requestedSceneName_ = sceneName; }

	// 現在のシーン名を取得
	const std::string& GetCurrentSceneName() const { return currentSceneName_; }

	// シーン更新
	void Update();

	// シーン描画
	void Draw();

	// 終了処理
	void Finalize();

private:
	void AllModelLoad();

private:
	// シーンマップ
	std::unordered_map<std::string, std::unique_ptr<BaseScene>> scenes_;

	// 現在のシーン
	BaseScene* currentScene_ = nullptr;

	// 現在のシーン名
	std::string currentSceneName_;

	// リクエストされたシーン名
	std::string requestedSceneName_;

	// 次のシーン名（遅延切り替え用）
	std::string nextSceneName_;

	// シーン切り替え中フラグ
	bool isChangingScene_ = false;

	// 共通リソースへのポインタ
	EngineContext* engineContext_ = nullptr;
	DirectInput* keyboard_ = nullptr;
	GamePad* gamePad_ = nullptr;
	DebugCamera* debugCamera_ = nullptr;
	DeltaTime* timeManager_ = nullptr;
};
