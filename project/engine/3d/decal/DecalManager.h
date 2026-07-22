#pragma once
#include "DebugCamera.h"
#include "EngineContext.h"
#include "MathUtility.h"
#include "ModelData.h"
#include "ParticleMaterial.h"
#include "Transform.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>

struct DecalForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

namespace TinyEngine {

/// <summary>
/// デカール管理クラス
/// </summary>
class DecalManager {
public:
	// デカールの最大描画数
	static const uint32_t kNumMaxInstance = 10000;

	// 登録されているデカールのデータ
	struct DecalData {
		Transform transform;
		Vector4 color;
		Matrix4x4 worldMatrix;
		std::string texturePath;
	};

	~DecalManager();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(EngineContext* ctx);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// デカールをステージ上に追加する
	/// </summary>
	DecalData* AddDecal(const std::string& texturePath, const Vector3& position, const Vector3& rotation, const Vector3& scale = {1.0f, 1.0f, 1.0f}, const Vector4& color = {1.0f, 1.0f, 1.0f, 1.0f});

	// Setter
	void SetCamera(Camera* camera) { camera_ = camera; }

private:
	ModelData CreatePrimitive();
	void CreateVertexData();

private:
	EngineContext* ctx_ = nullptr;
	Camera* camera_ = nullptr;

	// メッシュ・マテリアルデータ
	ModelData modelData_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	// インスタンシング用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	DecalForGPU* instancingData_ = nullptr;
	uint32_t srvIndex_ = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_{};
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_{};

	std::vector<DecalData> decalList_; // デカールの実データ配列
	uint32_t decalIndex_ = 0;          // リングバッファ用のインデックス
	uint32_t numInstance_ = 0;         // 現在の描画数

	// テクスチャごとの管理グループ
	struct DecalGroup {
		std::vector<DecalData> decalList;
		uint32_t decalIndex = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
		DecalForGPU* instancingData = nullptr;
		uint32_t srvIndex = 0;
		D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU{};
		D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU{};
	};

	// テクスチャパスをキーにしたグループのマップ
	std::unordered_map<std::string, std::unique_ptr<DecalGroup>> decalGroups_;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	ParticleMaterial* materialData_ = nullptr;
	ParticleMaterial material_;
};

} // namespace TinyEngine