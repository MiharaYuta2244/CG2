#include "Skybox.h"
#include "DirectXUtils.h"
#include "MathUtility.h"

using namespace TinyEngine;

void Skybox::Initialize(EngineContext* ctx, const std::string& textureFilePath) {
	ctx_ = ctx;
	textureFilePath_ = "resources/textures/" + textureFilePath;

	CreateVertexAndIndexData();
	CreateTransformationData();
}

void Skybox::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
	Matrix4x4 skyboxViewMatrix = viewMatrix;
	skyboxViewMatrix.m[3][0] = 0.0f;
	skyboxViewMatrix.m[3][1] = 0.0f;
	skyboxViewMatrix.m[3][2] = 0.0f;

	Matrix4x4 scaleMat = MathUtility::MakeScaleMatrix(scale_);
	Matrix4x4 translateMat = MathUtility::MakeTranslateMatrix(position_);
	Matrix4x4 worldMatrix = MathUtility::Multiply(scaleMat, translateMat);
	Matrix4x4 worldViewProjectionMatrix = MathUtility::Multiply(worldMatrix, MathUtility::Multiply(skyboxViewMatrix, projectionMatrix));

	*transformationMatrixData_ = {worldViewProjectionMatrix};
}

void Skybox::Draw() {
	ctx_->skyboxCommon->DrawSettingCommon();

	auto cmdList = ctx_->skyboxCommon->GetDirectXCommon()->GetCommandList();
	cmdList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	cmdList->IASetIndexBuffer(&indexBufferView_);

	cmdList->SetGraphicsRootConstantBufferView(0, transformMatrixResource_->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(1, ctx_->textureManager->GetSrvHandleGPU(textureFilePath_));

	cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

void Skybox::CreateVertexAndIndexData() {
	auto device = ctx_->skyboxCommon->GetDirectXCommon()->GetDevice();

	vertexResource_ = DirectXUtils::CreateBufferResource(device, sizeof(VertexData) * 8);
	indexResource_ = DirectXUtils::CreateBufferResource(device, sizeof(uint32_t) * 36);

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 8;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 36;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	vertexData[0].position = {-1.0f, -1.0f, 1.0f, 1.0f};  // 左下前
	vertexData[1].position = {1.0f, -1.0f, 1.0f, 1.0f};   // 右下前
	vertexData[2].position = {-1.0f, 1.0f, 1.0f, 1.0f};   // 左上前
	vertexData[3].position = {1.0f, 1.0f, 1.0f, 1.0f};    // 右上前
	vertexData[4].position = {-1.0f, -1.0f, -1.0f, 1.0f}; // 左下奥
	vertexData[5].position = {1.0f, -1.0f, -1.0f, 1.0f};  // 右下奥
	vertexData[6].position = {-1.0f, 1.0f, -1.0f, 1.0f};  // 左上奥
	vertexData[7].position = {1.0f, 1.0f, -1.0f, 1.0f};   // 右上奥

	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	// インデックスデータ
	uint32_t indices[] = {
	    // 前
	    0, 1, 2, 2, 1, 3,
	    // 奥
	    5, 4, 7, 7, 4, 6,
	    // 左
	    4, 0, 6, 6, 0, 2,
	    // 右
	    1, 5, 3, 3, 5, 7,
	    // 上
	    2, 3, 6, 6, 3, 7,
	    // 下
	    4, 5, 0, 0, 5, 1};
	for (int i = 0; i < 36; ++i) {
		indexData[i] = indices[i];
	}
}

void Skybox::CreateTransformationData() {
	transformMatrixResource_ = DirectXUtils::CreateBufferResource(ctx_->skyboxCommon->GetDirectXCommon()->GetDevice(), sizeof(TransformationMatrix));
	transformMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	*transformationMatrixData_ = {MathUtility::MakeIdentity4x4()};
}