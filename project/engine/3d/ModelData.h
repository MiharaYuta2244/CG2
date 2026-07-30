#pragma once
#include "MaterialData.h"
#include "Node.h"
#include "VertexData.h"
#include <vector>

struct VertexWeightData {
	float weight;
	uint32_t vertexIndex;
};

struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};

struct MeshInfo {
	uint32_t indexOffset;
	uint32_t indexCount;
	uint32_t materialIndex;
};

/// <summary>
/// モデルデータ構造体
/// </summary>
struct ModelData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	std::vector<MaterialData> materials;
	std::vector<MeshInfo> meshes;
	Node rootNode;
	std::map<std::string, JointWeightData> skinClusterData;
};