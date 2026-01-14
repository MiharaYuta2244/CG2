#pragma once
#include <wrl.h>
#include "Material.h"
#include "MaterialData.h"
#include "ModelData.h"
#include "ModelCommon.h"
#include "MeshData.h"
#include "IMeshGenerator.h"
#include <d3d12.h>
#include <string>
#include <vector>

class TextureManager;

class Model {
public:
	void Initialize(ModelCommon* modelCommon, TextureManager* textureManager, const std::string& filename);

	void Draw();

	void Update();

private:
	ModelData LoadModelFile(const std::string& filename);

	MaterialData LoadMaterialTemplateFile(const std::string& filename);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeBytes);

	/// <summary>
	/// 頂点データ作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	/// インデックスデータ作成
	/// </summary>
	void CreateIndexData();

	// assimpのNode(aiNode)から、右の構造体に変換する関数を作る
	//Node ReadNode(aiNode* node);

private: 
	ModelCommon* modelCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	std::string filename_;

	// objファイルのデータ
	ModelData modelData_;

	// メッシュデータ
	MeshData meshData_;

	// VertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	// Resource
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;

	// Resourceにデータを書き込むためのポインタ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;

	// インデックスカウント
	uint32_t indexCount_;
};
