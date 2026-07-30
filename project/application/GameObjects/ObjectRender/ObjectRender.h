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

	// Setter
	void SetColor(Vector4 color) { object3d_->SetColor(color); }
	void SetScale(Vector3 scale) { object3d_->SetScale(scale); }
	void SetTransform(Transform transform) { object3d_->SetTransform(transform); }
	void SetTranslate(Vector3 translate) { object3d_->SetTranslate(translate); }
	void SetEnvScale(float envScale) { object3d_->SetEnvScale(envScale); }
	void SetTexture(const std::string& filepath) { object3d_->SetTexture(filepath); }
	void SetEnableLighting(bool enableLighting) { object3d_->SetEnableLighting(enableLighting); }
	void SetEnableNoise(bool enableNoise) { object3d_->SetEnableNoise(enableNoise); }
	void SetTime(float time) { object3d_->SetTime(time); }
	void SetIsSkinning(bool isSkinning) { object3d_->SetIsSkinning(isSkinning); }
	void SetEnableLaser(bool enableLaser) { object3d_->SetEnableLaser(enableLaser); }
	void SetModel(const std::string& filepath) { object3d_->SetModel(filepath); }
	void SetEnableOutline(bool isEnable) {
		if (object3d_) {
			object3d_->SetEnableOutline(isEnable);
		}
	}

	// 描画用オブジェクトを返すGetter
	TinyEngine::Object3d* GetObject3d() { return object3d_.get(); }
	const std::string GetFilepath() { return object3d_->GetFilepath(); }
	Vector3 GetBonePos(const std::wstring& boneName) { return object3d_->GetBonePos(boneName); }

private:
	// モデル
	std::unique_ptr<TinyEngine::Object3d> object3d_;
};
