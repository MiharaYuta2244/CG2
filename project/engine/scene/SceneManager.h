#pragma once
#include "BaseScene.h"
#include <memory>
#include <unordered_map>
#include <vector>

// 遷移の種類
enum class SceneTransition { None, Change, Push, Pop };

/// <summary>
/// シーンマネージャークラス
/// </summary>
class SceneManager {
public:
	SceneManager() = default;
	~SceneManager() = default;

	// シーン初期化（共通リソースを渡す）
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* debugCamera, TimeManager* timeManager);

	// シーン追加
	void AddScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene);

	// シーン切り替え要求
	void ChangeScene(const std::string& sceneName);
	void PushScene(const std::string& sceneName);
	void PopScene();

	// 現在のシーンを取得
	BaseScene* GetCurrentScene() const;

	// 要求されているシーン名を取得
	std::string GetRequestedSceneName() const { return requestedSceneName_; }

	// 要求されているシーン名を設定
	void SetRequestedSceneName(const std::string& sceneName) { requestedSceneName_ = sceneName; }

	// 要求されている遷移の種類を取得
	SceneTransition GetRequestedTransition() const { return requestedTransition_; }

	// 要求されている遷移の種類をクリア
	void ClearRequest() {
		requestedSceneName_ = "";
		requestedTransition_ = SceneTransition::None;
	}

	// シーン更新
	void Update();

	// シーン描画
	void Draw();

	// 終了処理
	void Finalize();

private:
	// シーンマップ
	std::unordered_map<std::string, std::unique_ptr<BaseScene>> scenes_;

	std::vector<BaseScene*> sceneStack_;
	std::string requestedSceneName_;
	std::string nextSceneName_;

	// 切り替えフラグ
	SceneTransition requestedTransition_ = SceneTransition::None;

	// 共通リソースへのポインタ
	EngineContext* engineContext_ = nullptr;
	DirectInput* keyboard_ = nullptr;
	GamePad* gamePad_ = nullptr;
	Camera* debugCamera_ = nullptr;
	TimeManager* timeManager_ = nullptr;
};
