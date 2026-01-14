#pragma once
#include "Matrix4x4.h"
#include "Node.h"
#include <string>
#include <vector>

struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};