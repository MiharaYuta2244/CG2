#pragma once
#include "CameraForGPU.h"
#include "EngineContext.h"
#include "FogParam.h"
#include "Material.h"
#include "TimeParam.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>

namespace TinyEngine {

/// <summary>
/// 敵の視界（扇形）を描画するための単一クラス
/// </summary>
class VisionCone {
public:
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="ctx">エンジンコンテキスト</param>
	/// <param name="radius">視界の長さ</param>
	/// <param name="angleDegrees">視野角（度数法: 60.0f など）</param>
	/// <param name="segments">扇形の分割数</param>
	void Initialize(EngineContext* ctx, float radius, float angleDegrees, uint32_t segments = 24);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="texturePath">視界用テクスチャのパス</param>
	void Draw(const std::string& texturePath = "");

	// Setter
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetColor(const Vector4& color) { material_.color = color; }

	// Getter
	Transform& GetTransform() { return transform_; }
	Matrix4x4& GetWorldMatrix() { return worldMatrix_; }

private:
	/// <summary>
	/// 扇形メッシュの生成
	/// </summary>
	void CreateMesh(float radius, float angleDegrees, uint32_t segments);

	/// <summary>
	/// 各種定数バッファの生成
	/// </summary>
	void CreateConstantBuffers();

private:
	EngineContext* ctx_ = nullptr;

	// メッシュ用バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	uint32_t vertexCount_ = 0;
	uint32_t indexCount_ = 0;

	// 定数バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	// 定数バッファのマップ先ポインタ
	TransformationMatrix* transformMatrixData_ = nullptr;
	Material* materialData_ = nullptr;

	// トランスフォーム・マテリアルデータ
	Transform transform_;
	Matrix4x4 worldMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;
	Material material_;
};

} // namespace TinyEngine