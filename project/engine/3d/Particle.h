#pragma once
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>
#include "CameraForGPU.h"
#include "DirectionalLight.h"
#include "FogParam.h"
#include "Material.h"
#include "MaterialData.h"
#include "ModelData.h"
#include "ModelManager.h"
#include "TimeParam.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexData.h"

class ParticleCommon;
class TextureManager;
class Model;

class Particle {
public:
	void Initialize(ParticleCommon* particleCommon, TextureManager* textureManager, ModelManager* modelManager);

	void Update();

	void Draw();

	void SetModel(const std::string& filePath);
	void SetEnableFoging(const bool enableFoging);
	void SetEnableLighting(const bool enableLighting);
	void SetColor(Vector4 color);

	// setter
	void SetModel(Model* model) { model_ = model; }

	// setter
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetWorldMatrix(Matrix4x4 worldMatrix) { worldMatrix_ = worldMatrix; }
	void SetViewMatrix(Matrix4x4 viewMatrix) { viewMatrix_ = viewMatrix; }
	void SetProjectionmatrix(Matrix4x4 projectionMatrix) { projectionMatrix_ = projectionMatrix; }

	// getter
	const Vector3& GetScale() const { return transform_.scale; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetTranslate() const { return transform_.translate; }
	const Vector4& GetColor() const { return material_.color; }

	Matrix4x4& GetWorldMatrix() { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }

private:
	// .mtlファイルの読み取り
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	// .objファイルの読み取り
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeBytes);

	/// <summary>
	/// 座標変換行列データ作成
	/// </summary>
	void CreateTransformationMatrixData();

	/// <summary>
	/// インスタンシングデータの作成
	/// </summary>
	void CreateInstancingData();

private:
	ParticleCommon* particleCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	Model* model_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraForGPUResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> fogParamResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> timeParamResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData_ = nullptr;
	TransformationMatrix* transformMatrixData_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	CameraForGPU* cameraForGPUData_ = nullptr;
	FogParam* fogParamData_ = nullptr;
	TimeParam* timeParamData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	// データ変更用の変数
	Material material_;
	Matrix4x4 worldMatrix_;
	DirectionalLight directionalLight_;
	CameraForGPU cameraForGPU_;
	FogParam fogParam_;
	TimeParam timeParam_;

	// 3Dオブジェクト自体とカメラの座標変換行列の元となるTransform
	Transform transform_;
	Transform cameraTransform_;

	// ビューマトリックス
	Matrix4x4 viewMatrix_;

	// プロジェクションマトリックス
	Matrix4x4 projectionMatrix_;

	// 描画する数
	static const uint32_t kNumInstance = 10;
};
