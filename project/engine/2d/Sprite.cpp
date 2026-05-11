#include "Sprite.h"
#include "DirectXUtils.h"
#include "MathUtility.h"
#include "Matrix4x4.h"
#include "VertexData.h"
#include "WinApp.h"
#include <cassert>

using namespace Microsoft::WRL;
using namespace TinyEngine;

Sprite::~Sprite() {}

void Sprite::Initialize(EngineContext* ctx, std::string textureFilePath) {
	ctx_ = ctx;
	textureFilePath_ = "resources/textures/" + textureFilePath;

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

	textureIndex_ = ctx_->textureManager->GetSrvIndex(textureFilePath_);

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
	vertexData_[0].position = {left_ + vertexOffsets_[0].x, bottom_ + vertexOffsets_[0].y, zDepth_, 1.0f};  // 左下
	vertexData_[1].position = {left_ + vertexOffsets_[1].x, top_ + vertexOffsets_[1].y, zDepth_, 1.0f};     // 左上
	vertexData_[2].position = {right_ + vertexOffsets_[2].x, bottom_ + vertexOffsets_[2].y, zDepth_, 1.0f}; // 右下
	vertexData_[3].position = {right_ + vertexOffsets_[3].x, top_ + vertexOffsets_[3].y, zDepth_, 1.0f};    // 右上

	vertexData_[0].texcoord = {tex_left_, tex_bottom_};
	vertexData_[1].texcoord = {tex_left_, tex_top_};
	vertexData_[2].texcoord = {tex_right_, tex_bottom_};
	vertexData_[3].texcoord = {tex_right_, tex_top_};

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

	// 発光処理の更新
	UpdateShine();

	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrix = MathUtility::MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MathUtility::MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = MathUtility::Multiply(worldMatrix_, MathUtility::Multiply(viewMatrix, projectionMatrix));
	*transformationMatrixData_ = {worldViewProjectionMatrix};

	// uvTransform
	Matrix4x4 uvTransformMatrix = MathUtility::MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = MathUtility::Multiply(uvTransformMatrix, MathUtility::MakeRollRotateMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = MathUtility::Multiply(uvTransformMatrix, MathUtility::MakeTranslateMatrix(uvTransform_.translate));
	materialData_->uvTransform = uvTransformMatrix;
}

void Sprite::Draw() {
	// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	ctx_->spriteCommon->DrawSettingCommon();

	// Spriteの描画
	auto cmdList = ctx_->spriteCommon->GetDirectXCommon()->GetCommandList();
	cmdList->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
	cmdList->IASetIndexBuffer(&indexBufferView_);          // IBVを設定
	// TransformationMatrixCBufferの場所を設定
	cmdList->SetGraphicsRootConstantBufferView(1, transformMatrixResource_->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(2, ctx_->textureManager->GetSrvHandleGPU(textureFilePath_));
	// 描画! (DrawCall/ドローコール)
	cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::CreateVertexData() {
	auto device = ctx_->spriteCommon->GetDirectXCommon()->GetDevice();

	// スプライトの頂点リソースを作る
	vertexResource_ = DirectXUtils::CreateBufferResource(device, sizeof(VertexData) * 4);
	// インデックスリソースを作る
	indexResource_ = DirectXUtils::CreateBufferResource(device, sizeof(uint32_t) * 6);

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
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
}

void Sprite::CreateMaterialData() {
	// Sprite用のマテリアルリソースを作る
	materialResource_ = DirectXUtils::CreateBufferResource(ctx_->spriteCommon->GetDirectXCommon()->GetDevice(), sizeof(Material));
	materialData_ = nullptr;

	// Mapしてデータを書き込む
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	material_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	material_.uvTransform = MathUtility::MakeIdentity4x4();
	material_.shineParams = {0.0f, 0.5f, 1.0f, 1.0f};
	material_.shineColor = {1.0f, 1.0f, 1.0f, 1.0f};
	material_.voronoiParams = {10.0f, 2.0f, 1.0f, 0.0f}; // scale, speed, intensity, time
	material_.voronoiColor = {1.0f, 1.0f, 1.0f, 1.0f};
	material_.enableVoronoi = false;
	material_.enableShine = false;
	material_.gradientTopColor = {1.0f, 1.0f, 1.0f, 1.0f};
	material_.gradientBottomColor = {1.0f, 1.0f, 1.0f, 1.0f};
	material_.enableGradient = false;
	*materialData_ = material_;
}

void Sprite::CreateTransformationData() {
	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformMatrixResource_ = DirectXUtils::CreateBufferResource(ctx_->spriteCommon->GetDirectXCommon()->GetDevice(), sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	transformMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	// 単位行列を書き込んでおく
	*transformationMatrixData_ = {MathUtility::MakeIdentity4x4()};
}

void Sprite::SetSrvHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHandle) { srvHandle_ = srvHandle; }

void Sprite::SetTexture(const std::string& texturePath) {
	textureFilePath_ = "resources/textures/" + texturePath;
	textureIndex_ = ctx_->textureManager->GetSrvIndex(texturePath);
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
	const DirectX::TexMetadata& metadata = ctx_->textureManager->GetMetaData(textureFilePath_);
	tex_left_ = textureLeftTop_.x / metadata.width;
	tex_right_ = (textureLeftTop_.x + textureSize_.x) / metadata.width;
	tex_top_ = textureLeftTop_.y / metadata.height;
	tex_bottom_ = (textureLeftTop_.y + textureSize_.y) / metadata.height;
}

void Sprite::AdjustTextureSize() {
	// テクスチャデータを取得
	const DirectX::TexMetadata& metadata = ctx_->textureManager->GetMetaData(textureFilePath_);

	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);

	// 画像サイズをテクスチャサイズに合わせる
	size_ = textureSize_;
}

void Sprite::UpdateShine() {
	shineTimer_ += 1.0f / 60.0f;

	float cycle = 1.0f;
	float t = fmod(shineTimer_, cycle);

	if (t < 1.0f) {
		material_.shineParams.x = t;
	} else {
		material_.shineParams.x = -1.0f;
		shineTimer_ = 0.0f;
	}

	materialData_->shineParams.x = material_.shineParams.x;
}
