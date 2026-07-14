#pragma once
#include "DebugCamera.h"
#include "EngineContext.h"
#include "MathUtility.h"
#include "ModelData.h"
#include "Transform.h"
#include "ParticleMaterial.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>

struct BloodDecalForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

namespace TinyEngine {

/// <summary>
/// 血痕管理クラス
/// </summary>
class BloodDecalManager {
public:
	// 血痕の最大描画数
	static const uint32_t kNumMaxInstance = 10000;

	~BloodDecalManager();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(EngineContext* ctx, const std::string& texturePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// 血痕をステージ上に追加する
	/// </summary>
	void AddBlood(const Vector3& position, const Vector3& rotation, const Vector3& scale = {1.0f, 1.0f, 1.0f}, const Vector4& color = {0.6f, 0.0f, 0.0f, 1.0f});

	// Setter
	void SetCamera(Camera* camera) { camera_ = camera; }

private:
	ModelData CreatePrimitive(const std::string& texturePath);
	void CreateVertexData();
	void CreateInstancingResource();
	void CreateInstancingSRV(UINT srvIndex);

private:
	EngineContext* ctx_ = nullptr;
	Camera* camera_ = nullptr;

	// メッシュ・マテリアルデータ
	ModelData modelData_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	// インスタンシング用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	BloodDecalForGPU* instancingData_ = nullptr;
	uint32_t srvIndex_ = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_{};
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_{};

	// 登録されている血痕のデータ
	struct BloodData {
		Transform transform;
		Vector4 color;
		Matrix4x4 worldMatrix;
	};

	std::vector<BloodData> bloodList_; // 血痕の実データ配列
	uint32_t bloodIndex_ = 0;          // リングバッファ用のインデックス
	uint32_t numInstance_ = 0;         // 現在の描画数

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	ParticleMaterial* materialData_ = nullptr;
	ParticleMaterial material_;
};

} // namespace TinyEngine