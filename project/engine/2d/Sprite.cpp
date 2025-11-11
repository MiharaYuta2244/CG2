#include <cassert>
#include "Sprite.h"
#include "MathUtility.h"
#include "Matrix4x4.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "VertexData.h"
#include "WinApp.h"

using namespace Microsoft::WRL;

Sprite::~Sprite() {}

void Sprite::Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager, std::string textureFilePath) {
	spriteCommon_ = spriteCommon;
	textureManager_ = textureManager;
	textureFilePath_ = textureFilePath;

	// 頂点データ作成
	CreateVertexData();

	// マテリアルデータ作成
	CreateMaterialData();

	// 座標変換行列データ作成
	CreateTransformationData();

	transform_ = {
	    {size_.x,     size_.y,     1.0f     },
        {0.0f,        0.0f,        rotation_},
        {position_.x, position_.y, 0.0f     }
    };

	uvTransform_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	textureIndex_ = textureManager_->GetSrvIndex(textureFilePath_);

	// テクスチャサイズをイメージに合わせる
	AdjustTextureSize();
}

void Sprite::Update() {
	left_ = 0.0f - anchorPoint_.x;
	right_ = 1.0f - anchorPoint_.x;
	top_ = 0.0f - anchorPoint_.y;
	bottom_ = 1.0f - anchorPoint_.y;

	// フリップ反映
	ApplyFlip();

	// テクスチャ範囲指定
	TextureRangeSelection();

	// 頂点データ4つで四角形の描画
	vertexData_[0].position = {left_, bottom_, 0.0f, 1.0f};  // 左下
	vertexData_[1].position = {left_, top_, 0.0f, 1.0f};     // 左上
	vertexData_[2].position = {right_, bottom_, 0.0f, 1.0f}; // 右下
	vertexData_[3].position = {right_, top_, 0.0f, 1.0f};    // 右上

	vertexData_[0].texcoord = {tex_left_, tex_bottom_};
	vertexData_[1].texcoord = {tex_left_, tex_top_};
	vertexData_[2].texcoord = {tex_right_, tex_bottom_};
	vertexData_[3].texcoord = {tex_right_, tex_top_};

	// 書き込むためのアドレスを取得
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	indexResource_->Unmap(0, nullptr);

	indexData_[0] = 0;
	indexData_[1] = 1;
	indexData_[2] = 2;
	indexData_[3] = 1;
	indexData_[4] = 3;
	indexData_[5] = 2;

	// transform
	transform_ = {
	    {size_.x,     size_.y,     1.0f     },
        {0.0f,        0.0f,        rotation_},
        {position_.x, position_.y, 0.0f     }
    };

	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrix = MathUtility::MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MathUtility::MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = MathUtility::Multiply(worldMatrix_, MathUtility::Multiply(viewMatrix, projectionMatrix));
	*transformationMatrixData_ = {worldViewProjectionMatrix, worldMatrix_};

	// uvTransform
	Matrix4x4 uvTransformMatrix = MathUtility::MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = MathUtility::Multiply(uvTransformMatrix, MathUtility::MakeRollRotateMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = MathUtility::Multiply(uvTransformMatrix, MathUtility::MakeTranslateMatrix(uvTransform_.translate));
	materialData_->uvTransform = uvTransformMatrix;
}

void Sprite::Draw() {
	// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spriteCommon_->DrawSettingCommon();

	// Spriteの描画。
	spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);          // IBVを設定
	// TransformationMatrixCBufferの場所を設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformMatrixResource_->GetGPUVirtualAddress());
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(textureFilePath_));
	// 描画! (DrawCall/ドローコール)
	spriteCommon_->GetDirectXCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::CreateVertexData() {
	// スプライトの頂点リソースを作る
	vertexResource_ = CreateBufferResource(sizeof(VertexData) * 4);
	// インデックスリソースを作る
	indexResource_ = CreateBufferResource(sizeof(uint32_t) * 6);

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	vertexResource_->Unmap(0, nullptr);
}

void Sprite::CreateMaterialData() {
	// Sprite用のマテリアルリソースを作る
	materialResource_ = CreateBufferResource(sizeof(Material));
	materialData_ = nullptr;

	// Mapしてデータを書き込む
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialResource_->Unmap(0, nullptr);

	material_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	material_.enableLighting = false;
	material_.uvTransform = MathUtility::MakeIdentity4x4();
	*materialData_ = material_;
}

void Sprite::CreateTransformationData() {
	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformMatrixResource_ = CreateBufferResource(sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	transformMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformMatrixResource_->Unmap(0, nullptr);

	// 単位行列を書き込んでおく
	*transformationMatrixData_ = {MathUtility::MakeIdentity4x4(), worldMatrix_};
}

void Sprite::SetSrvHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHandle) { srvHandle_ = srvHandle; }

void Sprite::SetTexture(const std::string texturePath) { textureIndex_ = textureManager_->GetSrvIndex(texturePath); }

ComPtr<ID3D12Resource> Sprite::CreateBufferResource(size_t sizeBytes) {
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // uploadHeapを使う
	// 頂点にリソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Alignment = 0;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeBytes; // リソースのサイズ。今回はVector4を3頂点分
	// バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = spriteCommon_->GetDirectXCommon()->GetDevice()->CreateCommittedResource(
	    &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(resource.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return resource;
}

void Sprite::ApplyFlip() {
	// 左右反転
	if (isFlipX_) {
		left_ = -left_;
		right_ = -right_;
	}

	// 上下反転
	if (isFlipY_) {
		top_ = -top_;
		bottom_ = -bottom_;
	}
}

void Sprite::TextureRangeSelection() {
	const DirectX::TexMetadata& metadata = textureManager_->GetMetaData(textureFilePath_);
	tex_left_ = textureLeftTop_.x / metadata.width;
	tex_right_ = (textureLeftTop_.x + textureSize_.x) / metadata.width;
	tex_top_ = textureLeftTop_.y / metadata.height;
	tex_bottom_ = (textureLeftTop_.y + textureSize_.y) / metadata.height;
}

void Sprite::AdjustTextureSize() {
	// テクスチャデータを取得
	const DirectX::TexMetadata& metadata = textureManager_->GetMetaData(textureFilePath_);

	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);

	// 画像サイズをテクスチャサイズに合わせる
	size_ = textureSize_;
}
