#pragma once
#include "Object3d.h"

/// <summary>
/// オブジェクトの描画処理を行う基底クラス
/// </summary>
class ObjectRender {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, const std::string& filepath);

	// 更新処理
	void Update(Transform transform);

	// 描画処理
	void Draw();

	// 色のSetter
	void SetColor(Vector4 color) { object3d_->SetColor(color); }

	// TransformのSetter
	void SetTransform(Transform transform) { object3d_->SetTransform(transform); }

	// 環境マップにかけるスケールの設定
	void SetEnvScale(float envScale) { object3d_->SetEnvScale(envScale); }

	// 描画用オブジェクトを返すGetter
	TinyEngine::Object3d* GetObject3d() { return object3d_.get(); }

private:
	// モデル
	std::unique_ptr<TinyEngine::Object3d> object3d_;
};
