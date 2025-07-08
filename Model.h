#pragma once
#include "Material.h"
#include "MaterialData.h"
#include "ModelData.h"
#include "ModelCommon.h"
#include <d3d12.h>
#include <string>
#include <wrl.h>

class TextureManager;

class Model {
public:
	void Initialize(ModelCommon* modelCommon, TextureManager* textureManager);

	void Draw();

private:
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeBytes);

	/// <summary>
	/// 頂点データ作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	/// マテリアルデータ作成
	/// </summary>
	void CreateMaterialData();

private: 
	ModelCommon* modelCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;

	// objファイルのデータ
	ModelData modelData_;

	// VertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	// Rsource
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	// Resourceにデータを書き込むためのポインタ
	VertexData* vertexData_ = nullptr;
	Material* materialData_ = nullptr;

	// データ変更用の変数
	Material material_;
};
