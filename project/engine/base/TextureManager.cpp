#include "TextureManager.h"
#include "DirectXCommon.h"
#include "DirectXUtils.h"
#include "SrvManager.h"
#include "StringUtility.h"
#include "d3dx12.h"
#include <filesystem>
#include <vector>

uint32_t TextureManager::kSRVIndexTop = 1;

namespace fs = std::filesystem;
using namespace Microsoft::WRL;

TextureManager::~TextureManager() {}

void TextureManager::Initialize(DirectXCommon* directXCommon, SrvManager* srvManager) {
	directXCommon_ = directXCommon;
	srvManager_ = srvManager;

	// SRVの数と同数
	textureDatas_.reserve(DirectXCommon::kMaxSRVCount);

	// Textureを読んで転送する
	AllTextureLoad();
}

void TextureManager::LoadTexture(const std::string& filePath) {
	std::string fullPath;

	// すでに "resources/models/" が含まれているかチェック
	if (filePath.find("resources/models/") == std::string::npos) {
		fullPath = "resources/textures/" + filePath;
	} else {
		fullPath = filePath;
	}

	// 読み込み済みテクスチャを検索
	if (textureDatas_.contains(fullPath)) {
		return;
	}

	// テクスチャ枚数上限チェック
	assert(srvManager_->CheckTextureMax());

	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(fullPath);
	HRESULT hr;
	if (filePathW.ends_with(L".dds")) {
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(hr));

	// ミニマップの作成
	DirectX::ScratchImage mipImages{};
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	} else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	}
	assert(SUCCEEDED(hr));

	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas_[fullPath];

	// テクスチャデータの要素数番号をSRVのインデックスとする
	textureData.srvIndex = srvManager_->Allocate();

	// メタデータを保存し、Resourceを作成してデータをアップロードする
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureData.metaData = metadata;

	// Resourceの作成
	textureData.resource = CreateTextureResource(metadata);

	// SRVの設定を行う
	if (metadata.miscFlags & DirectX::TEX_MISC_TEXTURECUBE) {
		// Cube
		srvManager_->CreateSRVforTextureCube(textureData.srvIndex, textureData.resource.Get(), textureData.metaData.format, UINT(textureData.metaData.mipLevels));
	} else {
		// 通常の2Dテクスチャ
		srvManager_->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metaData.format, UINT(textureData.metaData.mipLevels));
	}
	// SRVを作成するDescriptorHeapの場所を決める
	// 先頭はImGuiが使っているのでその次を使う
	textureData.srvHandleCPU = directXCommon_->GetSRVCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = directXCommon_->GetSRVGPUDescriptorHandle(textureData.srvIndex);

	// 実データをGPUに書き込む
	ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(textureData.resource, mipImages, directXCommon_->GetDevice(), directXCommon_->GetCommandList());

	// コマンドを実行して完了を待つ
	directXCommon_->ExecuteCommandListAndWait();
}

void TextureManager::AllTextureLoad() {
	std::string directoryPath = "resources/textures/";

	for (const auto& entry : fs::directory_iterator(directoryPath)) {
		if (entry.is_regular_file()) {
			// 拡張子チェック
			auto ext = entry.path().extension().string();
			if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".dds" || ext == ".tga") {
				// ファイル名を取得して読み込む
				LoadTexture(entry.path().filename().string());
			}
		}
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath) {
	// 範囲外指定違反チェック
	assert(textureDatas_[filePath].srvIndex < textureDatas_.size() + 1);

	// テクスチャデータの参照を取得
	TextureData& textureData = textureDatas_[filePath];
	return textureData.srvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
	// 範囲外指定違反チェック
	assert(textureDatas_[filePath].srvIndex < textureDatas_.size() + 1);

	TextureData& textureData = textureDatas_[filePath];
	return textureData.metaData;
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath) { return textureDatas_[filePath].srvIndex; }

ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);                             // Textureの幅
	resourceDesc.Height = UINT(metadata.height);                           // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);                   // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);            // 奥行き or 配列Textureの配列数
	resourceDesc.Format = metadata.format;                                 // TextureのFormat
	resourceDesc.SampleDesc.Count = 1;                                     // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数。普段使っているのは2次元
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	// 非カスタムヒープでは以下を UNKNOWN にする必要がある
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	// Resourceの生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = directXCommon_->GetDevice()->CreateCommittedResource(
	    &heapProperties,                // Heapの設定
	    D3D12_HEAP_FLAG_NONE,           // Heapの特殊な設定。特になし。
	    &resourceDesc,                  // Resourceの設定
	    D3D12_RESOURCE_STATE_COPY_DEST, // 初回のResourceState。Textureは基本読むだけ
	    nullptr,                        // Clear最速値。使わないのでnullptr
	    IID_PPV_ARGS(&resource));       // 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}

[[nodiscard]]
ComPtr<ID3D12Resource>
    TextureManager::UploadTextureData(ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);

	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));

	ComPtr<ID3D12Resource> intermediateResource = DirectXUtils::CreateBufferResource(device, intermediateSize);

	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;

	commandList->ResourceBarrier(1, &barrier);

	return intermediateResource;
}