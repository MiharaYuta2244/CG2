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
#include "DebugCamera.h"

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

	Matrix4x4& GetWorldMatrix() { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeBytes);

	/// <summary>
	/// 座標変換行列データ作成
	/// </summary>
	void CreateTransformationMatrixData();

	/// <summary>
	/// インスタンシングデータの作成
	/// </summary>
	void CreateInstancingData();

	/// <summary>
	/// 頂点バッファの作成
	/// </summary>
	void CreateVertexBuffer();

private:
	ParticleCommon* particleCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	// transform 用の CBV リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformConstantBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraForGPUResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> fogParamResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> timeParamResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	// 頂点バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformMatrixData_ = nullptr;
	VertexData* vertexData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	// 頂点数
	uint32_t vertexCount_ = 0;

	// データ変更用の変数
	Matrix4x4 worldMatrix_;

	// 3Dオブジェクト自体とカメラの座標変換行列の元となるTransform
	Transform transform_;

	// ビューマトリックス
	Matrix4x4 viewMatrix_;

	// プロジェクションマトリックス
	Matrix4x4 projectionMatrix_;

	// 描画する数
	static const uint32_t kNumInstance = 10;

	// カメラ
	DebugCamera* camera_ = nullptr;

	Matrix4x4 worldViewProjectionMatrix_;
};
