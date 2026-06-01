#pragma once
#include "Matrix4x4.h"
#include "Node.h"
#include "QuaternionTransform.h"
#include <string>
#include <vector>

/// <summary>
/// GPUに送るノードの構造体
/// </summary>
struct Node {
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};