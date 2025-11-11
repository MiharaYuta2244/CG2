#include "Particle.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "Model.h"
#include "ParticleCommon.h"
#include "TextureManager.h"
#include <DirectXMath.h>
#include <fstream>
#include <sstream>

using namespace Microsoft::WRL;
using namespace DirectX;

void Particle::Initialize(ParticleCommon* particleCommon, TextureManager* textureManager, ModelManager* modelManager) {
	particleCommon_ = particleCommon;
	textureManager_ = textureManager;
	modelManager_ = modelManager;

	// 座標変換行列データ作成
	CreateTransformationMatrixData();

	// インスタンシングデータ作成
	CreateInstancingData();

	// 頂点バッファ作成（ModelDataに基づく6頂点）
	CreateVertexBuffer();

	// Transform変数を作る
	transform_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// カメラをセットする
	camera_ = particleCommon_->GetDefaultCamera();
}

void Particle::Update() {
	worldMatrix_ = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix_ = MathUtility::Multiply(worldMatrix_, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix_ = worldMatrix_;
	}

	transformMatrixData_->WVP = worldViewProjectionMatrix_;
	transformMatrixData_->World = worldMatrix_;

	*transformMatrixData_ = {transformMatrixData_->WVP, transformMatrixData_->World};
}

void Particle::Draw() {
	// Particle描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	particleCommon_->DrawSettingCommon();

	auto cmd = particleCommon_->GetDxCommon()->GetCommandList();

	// ルートシグネチャとPSOは既に設定する共通関数があれば呼ぶ
	particleCommon_->DrawSettingCommon();

	// DescriptorHeap をコマンドリストにセット（SRVヒープ）
	ID3D12DescriptorHeap* heaps[] = {particleCommon_->GetDxCommon()->GetSrvDescriptorHeap().Get()};
	cmd->SetDescriptorHeaps(_countof(heaps), heaps);

	// ルートパラメータのバインド
	// rootParameters[1] が Vertex 用 CBV ならここでバインド（例: インスタンス変換を入れるCB）
	cmd->SetGraphicsRootConstantBufferView(1, transformConstantBuffer_->GetGPUVirtualAddress());

	// rootParameters[2] にインスタンシング用 SRV テーブルをバインド
	cmd->SetGraphicsRootDescriptorTable(2, particleCommon_->GetInstancingSrvGpuHandle());

	// 頂点バッファ等の設定（既存の処理）
	cmd->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// インスタンス数は SRV の要素数（kNumInstance）と合わせる
	const UINT instanceCount = kNumInstance;
	cmd->DrawInstanced(UINT(vertexCount_), instanceCount, 0, 0);
}

ComPtr<ID3D12Resource> Particle::CreateBufferResource(ComPtr<ID3D12Device> device, size_t sizeBytes) {
	if (!device)
		return nullptr;

	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // uploadHeapを使う
	// 頂点にリソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeBytes; // リソースのサイズ。今回はVector4を3頂点分
	// バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

void Particle::CreateTransformationMatrixData() {
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	// ここで transformConstantBuffer_ を作成して、transformMatrixData_ をマップする
	transformConstantBuffer_ = CreateBufferResource(particleCommon_->GetDxCommon()->GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	transformConstantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));
	transformConstantBuffer_->Unmap(0, nullptr);
	// 単位行列を書き込んでおく
	*transformMatrixData_ = {MathUtility::MakeIdentity4x4(), worldMatrix_};
}

void Particle::CreateInstancingData() {
	Transform transforms[kNumInstance];

	for (uint32_t index = 0; index < kNumInstance; ++index) {
		transforms[index].scale = {1.0f, 1.0f, 1.0f};
		transforms[index].rotate = {0.0f, 0.0f, 0.0f};
		transforms[index].translate = {index * 0.1f, index * 0.1f, index * 0.1f};
	}

	// Instancing用のTransformationMatrixリソースを作る
	instancingResource_ = CreateBufferResource(particleCommon_->GetDxCommon()->GetDevice(), sizeof(TransformationMatrix) * kNumInstance);

	// 書き込むためのアドレスを取得
	TransformationMatrix* instancingData = nullptr;
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));
	instancingResource_->Unmap(0, nullptr);

	for (uint32_t index = 0; index < kNumInstance; ++index) {
		Matrix4x4 worldMatrix = MathUtility::MakeAffineMatrix(transforms[index].scale, transforms[index].rotate, transforms[index].translate);
		Matrix4x4 viewProjectionMatrix = viewMatrix_ * projectionMatrix_;
		Matrix4x4 worldViewProjectionMatrix = MathUtility::Multiply(worldMatrix, viewProjectionMatrix);
		instancingData[index].WVP = worldViewProjectionMatrix;
		instancingData[index].World = worldMatrix;
	}
}

void Particle::CreateVertexBuffer() {
	// ModelData に基づく 6 頂点（矩形=三角形2つ）
	std::vector<VertexData> vertices;
	vertices.push_back({
	    .position = {-1.0f, 1.0f, 0.0f, 1.0f},
          .texcoord = {0.0f, 0.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 左上
	vertices.push_back({
	    .position = {1.0f, 1.0f, 0.0f, 1.0f},
          .texcoord = {1.0f, 0.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 右上
	vertices.push_back({
	    .position = {-1.0f, -1.0f, 0.0f, 1.0f},
          .texcoord = {0.0f, 1.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 左下
	vertices.push_back({
	    .position = {-1.0f, -1.0f, 0.0f, 1.0f},
          .texcoord = {0.0f, 1.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 左下
	vertices.push_back({
	    .position = {1.0f, 1.0f, 0.0f, 1.0f},
          .texcoord = {1.0f, 0.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 右上
	vertices.push_back({
	    .position = {1.0f, -1.0f, 0.0f, 1.0f},
          .texcoord = {1.0f, 1.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 右下

	vertexCount_ = static_cast<uint32_t>(vertices.size());

	// 頂点バッファリソース作成
	vertexResource_ = CreateBufferResource(particleCommon_->GetDxCommon()->GetDevice(), sizeof(VertexData) * vertexCount_);

	// マップしてデータを書き込む
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, vertices.data(), sizeof(VertexData) * vertexCount_);
	vertexResource_->Unmap(0, nullptr);

	// ビューを作る
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertexCount_);
	vertexBufferView_.StrideInBytes = static_cast<UINT>(sizeof(VertexData));
}
