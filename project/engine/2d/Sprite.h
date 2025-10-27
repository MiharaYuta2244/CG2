#pragma once

#include <d3d12.h>
#include <memory>
#include <string>
#include <wrl.h>
#include "Material.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Vector2.h"

class SpriteCommon;
class TextureManager;

class Sprite {
public:
	~Sprite();
	void Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager, std::string textureFilePath);

	void Update();

	void Draw();

	/// <summary>
	/// 頂点データ作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	/// マテリアルデータ作成
	/// </summary>
	void CreateMaterialData();

	/// <summary>
	/// 座標変換行列データ作成
	/// </summary>
	void CreateTransformationData();

	// getter
	Vector2& GetPosition() { return position_; }
	float& GetRotation() { return rotation_; }
	Vector4& GetColor() { return materialData_->color; }
	Vector2& GetSize() { return size_; }
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() { return vertexBufferView_; }
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetTransformMatrixResource() { return transformMatrixResource_; }
	Vector2& GetAnchorPoint() { return anchorPoint_; }
	bool& GetIsFlipX() { return isFlipX_; }
	bool& GetIsFlipY() { return isFlipY_; }
	Vector2& GetTextureLeftTop() { return textureLeftTop_; }
	Vector2& GetTextureSize() { return textureSize_; }

	// setter
	void SetPosition(const Vector2& position) { position_ = position; }
	void SetRotation(float rotation) { rotation_ = rotation; }
	void SetColor(const Vector4& color) { materialData_->color = color; }
	void SetSize(const Vector2& size) { size_ = size; }
	void SetSrvHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHandle);
	void SetTexture(const std::string texturePath);
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }
	void SetIsFlipX(const bool& isFlipX) { isFlipX_ = isFlipX; }
	void SetIsFlipY(const bool& isFlipY) { isFlipY_ = isFlipY; }
	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }
	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }

private:
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeBytes);

	// フリップの反映処理
	void ApplyFlip();

	// テクスチャ範囲指定
	void TextureRangeSelection();

	// テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

private:
	SpriteCommon* spriteCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformMatrixResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;
	Material* materialData_ = nullptr;

	// バッファリソース内のデータを指すポインタ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle_;

	Transform transform_;
	Transform uvTransform_;
	Matrix4x4 worldMatrix_;
	Material material_;

	// Spriteの座標,回転,サイズ
	Vector2 position_ = {0.0f, 0.0f};
	float rotation_ = 0.0f;
	Vector2 size_ = {640.0f, 360.0f};

	// テクスチャ番号
	uint32_t textureIndex_ = 0;

	// アンカーポイント
	Vector2 anchorPoint_ = {0.0f, 0.0f};

	// 左右フリップ
	bool isFlipX_ = false;

	// 上下フリップ
	bool isFlipY_ = false;

	float left_;
	float right_;
	float top_;
	float bottom_;

	// テクスチャ範囲
	float tex_left_;
	float tex_right_;
	float tex_top_;
	float tex_bottom_;

	// テクスチャ左上座標
	Vector2 textureLeftTop_ = {0.0f, 0.0f};

	// テクスチャ切り出しサイズ
	Vector2 textureSize_ = {100.0f, 100.0f};
};
