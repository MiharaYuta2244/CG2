#pragma once
#include <wrl.h>
#include "Material.h"
#include "MaterialData.h"
#include "ModelData.h"
#include "ModelCommon.h"
#include "MeshData.h"
#include "Node.h"
#include "KeyframeAnimation.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <assimp/scene.h>

class TextureManager;

struct Joint {
    QuaternionTransform transform;
    Matrix4x4 localMatrix;
    Matrix4x4 skeletonSpaceMatrix; // 親の行列を乗算したグローバル行列
    std::string name;
    std::vector<int32_t> children;
    int32_t index;
    std::optional<int32_t> parent;
};

struct Skeleton {
    int32_t root;
    std::map<std::string, int32_t> jointMap; // 名前からインデックスを引くマップ
    std::vector<Joint> joints;
};

/// <summary>
/// モデルクラス
/// </summary>
class Model {
public:
	// 初期化関数
	void Initialize(ModelCommon* modelCommon, TextureManager* textureManager, const std::string& filename);

	// 描画関数
	void Draw(const std::string& textureFilePath = "");

	// 更新関数
	void Update();

	// Getter
	const ModelData& GetModelData() const { return modelData_; }

	Skeleton CreateSkeleton(const Node& rootNode);

	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);

	void UpdateSkeleton(Skeleton& skeleton);

	void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);

private:
	/// <summary>
	/// モデルファイルを読み込む
	/// </summary>
	/// <param name="filename">モデルファイルのパス</param>
	/// <returns>読み込んだモデルデータ</returns>
	ModelData LoadModelFile(const std::string& filename);

	/// <summary>
	/// マテリアルテンプレートファイルを読み込む
	/// </summary>
	/// <param name="filename">マテリアルテンプレートファイルのパス</param>
	/// <returns>読み込んだマテリアルデータ</returns>
	MaterialData LoadMaterialTemplateFile(const std::string& filename);

	/// <summary>
	/// 頂点データ作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	/// インデックスデータ作成
	/// </summary>
	void CreateIndexData();

	// assimpのNode(aiNode)から、右の構造体に変換する関数を作る
	Node ReadNode(aiNode* node);

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
