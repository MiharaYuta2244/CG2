#pragma once
#include "BaseScene.h"
#include "EasingEditor.h"
#include "Object3d.h"

class EasingEditorScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// イージングエディター
	std::unique_ptr<EasingEditor> easingEditor_;

	// 挙動確認用のモデル
	std::unique_ptr<TinyEngine::Object3d> object_;
};
