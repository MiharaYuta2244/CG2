#pragma once
#include "EngineContext.h"
#include "Matrix4x4.h"
#include "Vector4.h"
#include <d3d12.h>
#include <string>
#include <wrl.h>

namespace TinyEngine {
class Skybox {
public:
	void Initialize(EngineContext* ctx, const std::string& textureFilePath);
	void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix);
	void Draw();

	void SetPosition(const Vector3& pos) { position_ = pos; }
	void SetScale(const Vector3& scale) { scale_ = scale; }

	Vector3 GetPosition() const { return position_; }
	Vector3 GetScale() const { return scale_; }

private:
	void CreateVertexAndIndexData();
	void CreateTransformationData();

private:
	struct VertexData {
		Vector4 position;
	};

	struct TransformationMatrix {
		Matrix4x4 WVP;
	};

	EngineContext* ctx_ = nullptr;
	std::string textureFilePath_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformMatrixResource_;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	TransformationMatrix* transformationMatrixData_ = nullptr;

	Vector3 position_ = {0.0f, 0.0f, 0.0f};
	Vector3 scale_ = {1.0f, 1.0f, 1.0f};
};
} // namespace TinyEngine