#pragma once

#include "EngineContext.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Vector2.h"
#include "Vector4.h"
#include <d3d12.h>
#include <memory>
#include <string>
#include <wrl.h>

class Sprite {
public:
	~Sprite();
	void Initialize(EngineContext* ctx, std::string textureFilePath);

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
	float& GetZDepth() { return zDepth_; }

	// setter
	void SetPosition(const Vector2& position) { position_ = position; }
	void SetRotation(float rotation) { rotation_ = rotation; }
	void SetColor(const Vector4& color) { materialData_->color = color; }
	void SetSize(const Vector2& size) { size_ = size; }
	void SetSrvHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHandle);
	void SetTexture(const std::string& texturePath);
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }
	void SetIsFlipX(const bool& isFlipX) { isFlipX_ = isFlipX; }
	void SetIsFlipY(const bool& isFlipY) { isFlipY_ = isFlipY; }
	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }
	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }
	void SetEnableShine(const uint32_t& enableShine) { materialData_->enableShine = enableShine; }
	void SetShineColor(const Vector4& shineColor) { materialData_->shineColor = shineColor; }
	void SetShineParams(const Vector4& shineParams) { materialData_->shineParams = shineParams; }
	void SetZDepth(float zDepth) { zDepth_ = zDepth; }

	// 頂点ごとのオフセットを設定
	void SetVertexOffset(uint32_t index, const Vector2& offset) {
		if (index < 4)
			vertexOffsets_[index] = offset;
	}

	// オフセットをまとめてセットする
	void SetVertexOffsets(const Vector2 offsets[4]) {
		for (int i = 0; i < 4; ++i)
			vertexOffsets_[i] = offsets[i];
	}

private:
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	struct TransformationMatrix {
		Matrix4x4 WVP;
	};

	struct Material {
		Vector4 color;
		Matrix4x4 uvTransform;
		Vector4 shineParams;
		Vector4 shineColor;
		int32_t enableShine;
	};

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeBytes);

	// フリップの反映処理
	void ApplyFlip();

	// テクスチャ範囲指定
	void TextureRangeSelection();

	// テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

	// 発光更新処理
	void UpdateShine();

private:
	std::string textureFilePath_;

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

	// コンテキスト構造体
	EngineContext* ctx_;

	// 発光処理タイマー
	float shineTimer_ = 0.0f;

	// 頂点オフセット
	Vector2 vertexOffsets_[4] = {
	    {0.0f, 0.0f},
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {0.0f, 0.0f}
    };

	// 深度値
	float zDepth_ = 0.0f;
};
