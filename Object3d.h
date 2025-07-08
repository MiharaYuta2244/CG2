#pragma once
#include "CameraForGPU.h"
#include "DirectionalLight.h"
#include "FogParam.h"
#include "Material.h"
#include "MaterialData.h"
#include "ModelData.h"
#include "TimeParam.h"
#include "TransformationMatrix.h"
#include "VertexData.h"
#include "Transform.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>

class Object3dCommon;
class TextureManager;
class Model;

class Object3d {
public:
	void Initialize(Object3dCommon* modelCommon, TextureManager* textureManager);

	void Update();

	void Draw();

	// setter
	void SetModel(Model* model) { model_ = model; }

	// setter
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

	// getter
	const Vector3& GetScale() const { return transform_.scale; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetTranslate() const { return transform_.translate; }

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
	/// 平行光源データ作成
	/// </summary>
	void CreateDirectionalLightData();

	/// <summary>
	/// CameraForGPUデータの作成
	/// </summary>
	void CreateCameraForGPUData();

	/// <summary>
	/// フォグパラメータデータの作成
	/// </summary>
	void CreateFogParamData();

	/// <summary>
	/// タイムパラメータデータの作成
	/// </summary>
	void CreateTimeParamData();

private:
	Object3dCommon* object3dCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	Model* model_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraForGPUResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> fogParamResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> timeParamResource_;

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
};
