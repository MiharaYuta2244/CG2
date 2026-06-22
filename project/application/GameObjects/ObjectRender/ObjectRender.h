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
	void Update();

	// 描画処理
	void Draw();

	// 色のSetter
	void SetColor(Vector4 color) { object3d_->SetColor(color); }

	// TransformのSetter
	void SetTransform(Transform transform) { object3d_->SetTransform(transform); }

	void SetTranslate(Vector3 translate) { object3d_->SetTranslate(translate); }

	// 環境マップにかけるスケールの設定
	void SetEnvScale(float envScale) { object3d_->SetEnvScale(envScale); }

	// TextureのSetter
	void SetTexture(const std::string& filepath) { object3d_->SetTexture(filepath); }

	void SetEnableLighting(bool enableLighting) { object3d_->SetEnableLighting(enableLighting); }
	void SetEnableNoise(bool enableNoise) { object3d_->SetEnableNoise(enableNoise); }
	void SetTime(float time) { object3d_->SetTime(time); }

	// 描画用オブジェクトを返すGetter
	TinyEngine::Object3d* GetObject3d() { return object3d_.get(); }

private:
	// モデル
	std::unique_ptr<TinyEngine::Object3d> object3d_;
};
