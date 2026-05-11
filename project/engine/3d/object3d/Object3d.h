#pragma once
#include "CameraForGPU.h"
#include "DebugCamera.h"
#include "EngineContext.h"
#include "FogParam.h"
#include "Material.h"
#include "Model.h"
#include "ModelData.h"
#include "Outline.h"
#include "TimeParam.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include <ImGuizmo.h>
#include <d3d12.h>
#include <string>
#include <wrl.h>

namespace TinyEngine {
/// <summary>
/// 3Dオブジェクトクラス
/// </summary>
class Object3d {
public:
	/// <summary>
	/// 初期化関数
	/// </summary>
	/// <param name="ctx">エンジンコンテキスト</param>
	void Initialize(EngineContext* ctx, const std::string& name = "Object");

	/// <summary>
	/// 更新関数
	/// </summary>
	void Update();

	/// <summary>
	/// 描画関数
	/// </summary>
	void Draw();

	/// <summary>
	/// ギズモの描画
	/// </summary>
	/// <param name="camera">カメラ</param>
	/// <param name="operation">SRTのどれか</param>
	/// <param name="mode">扱う座標系</param>
	void DrawGizmo(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode);

	// setter
	void SetModel(const std::string& filePath);
	void SetEnableFoging(const bool enableFoging) { material_.enableFoging = enableFoging; }
	void SetEnableLighting(const bool enableLighting) { material_.enableLighting = enableLighting; }
	void SetColor(Vector4 color);
	void SetModel(Model* model) { model_ = model; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetWorldMatrix(Matrix4x4 worldMatrix) { worldMatrix_ = worldMatrix; }
	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetOutlineColor(const Vector4& color) { outline_.color = color; }
	void SetOutlineColor(float thickness) { outline_.thickness = thickness; }
	void SetName(const std::string& name) { name_ = name; }

	// getter
	Vector3& GetScale() { return transform_.scale; }
	Vector3& GetRotate() { return transform_.rotate; }
	Vector3& GetTranslate() { return transform_.translate; }
	Vector4& GetColor() { return material_.color; }
	Transform& GetTransform() { return transform_; }
	Matrix4x4& GetWorldMatrix() { return worldMatrix_; }
	Material& GetMaterial() { return material_; }
	uint32_t GetID() const { return id_; }
	const std::string& GetName() const { return name_; }

private:
	/// <summary>
	/// 座標変換行列データ作成
	/// </summary>
	void CreateTransformationMatrixData();

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

	/// <summary>
	/// マテリアルデータの作成（インスタンス毎に持つ）
	/// </summary>
	void CreateMaterialData();

	/// <summary>
	/// アウトラインデータの作成
	/// </summary>
	void CreateOutlineData();

private:
	Model* model_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraForGPUResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> fogParamResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> timeParamResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> outlineResource_;

	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData_ = nullptr;
	TransformationMatrix* transformMatrixData_ = nullptr;
	CameraForGPU* cameraForGPUData_ = nullptr;
	FogParam* fogParamData_ = nullptr;
	TimeParam* timeParamData_ = nullptr;
	Material* materialData_ = nullptr;
	Outline* outlineData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	// データ変更用の変数
	Material material_;
	Matrix4x4 worldMatrix_;
	CameraForGPU cameraForGPU_;
	FogParam fogParam_;
	TimeParam timeParam_;
	Outline outline_ = {
	    {0.0f, 0.0f, 0.0f, 1.0f},
        0.005f
    };

	// 3Dオブジェクト自体とカメラの座標変換行列の元となるTransform
	Transform transform_;

	Matrix4x4 worldViewProjectionMatrix_;

	// 描画する数
	static const uint32_t kNumInstance = 10;

	// カメラ
	Camera* camera_ = nullptr;

	// コンテキスト構造体
	EngineContext* ctx_;

	// モデルデータ
	ModelData modelData_;

	// オブジェクト識別用のID
	uint32_t id_;

	// オブジェクト識別用の名前
	std::string name_;

	// 全Object3dで共有する次に割り当てるID
	static uint32_t s_nextID_;
};
} // namespace TinyEngine