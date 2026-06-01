#pragma once
#include "VertexData.h"
#include "MaterialData.h"
#include "Node.h"
#include <vector>

/// <summary>
/// モデルデータ構造体
/// </summary>
struct ModelData
{
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	MaterialData material;
	Node rootNode;
};