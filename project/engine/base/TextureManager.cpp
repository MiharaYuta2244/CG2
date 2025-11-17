#include "TextureManager.h"
#include "DirectXCommon.h"
#include "StringUtility.h"
#include "SrvManager.h"

uint32_t TextureManager::kSRVIndexTop = 1;

using namespace Microsoft::WRL;

TextureManager::~TextureManager() {}

void TextureManager::Initialize(DirectXCommon* directXCommon, SrvManager* srvManager) {
	directXCommon_ = directXCommon;
	srvManager_ = srvManager;

	// SRVの数と同数
	textureDatas_.reserve(DirectXCommon::kMaxSRVCount);

	// Textureを読んで転送する
	LoadTexture("resources/uvChecker.png");
	LoadTexture("resources/white.png");
	LoadTexture("resources/fence.png");
	LoadTexture("resources/SkySphere.png");
	LoadTexture("resources/skydome.png");
	LoadTexture("resources/Field.png");
	LoadTexture("resources/sphere.png");
	LoadTexture("resources/Heart.png");
	LoadTexture("resources/monsterBall.png");
	LoadTexture("resources/circle.png");
}

void TextureManager::LoadTexture(const std::string& filePath) {
	// 読み込み済みテクスチャを検索
	if (textureDatas_.contains(filePath)) {
		return;
	}

	// テクスチャ枚数上限チェック
	assert(srvManager_->CheckTextureMax());

	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミニマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas_[filePath];

	// テクスチャデータの要素数番号をSRVのインデックスとする
	textureData.srvIndex = srvManager_->Allocate();

	// メタデータを保存し、Resourceを作成してデータをアップロードする
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureData.metaData = metadata;

	// Resourceの作成
	textureData.resource = CreateTextureResource(metadata);

	// 実データをGPUに書き込む
	UploadTextureData(textureData.resource, mipImages);

	// SRVを作成するDescriptorHeapの場所を決める
	// 先頭はImGuiが使っているのでその次を使う
	textureData.srvHandleCPU = directXCommon_->GetSRVCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = directXCommon_->GetSRVGPUDescriptorHandle(textureData.srvIndex);

	// SRVの設定を行う（ここでは初期化済みの resource と metaData を渡す）
	srvManager_->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metaData.format, UINT(textureData.metaData.mipLevels));
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

	// 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケースがある
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;                        // 細かい設定を行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUにアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;          // プロセッサの近くに配置

	// Resourceの生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = directXCommon_->GetDevice()->CreateCommittedResource(
	    &heapProperties,                   // Heapの設定
	    D3D12_HEAP_FLAG_NONE,              // Heapの特殊な設定。特になし。
	    &resourceDesc,                     // Resourceの設定
	    D3D12_RESOURCE_STATE_GENERIC_READ, // 初回のResourceState。Textureは基本読むだけ
	    nullptr,                           // Clear最速値。使わないのでnullptr
	    IID_PPV_ARGS(&resource));          // 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}

void TextureManager::UploadTextureData(ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages) {
	// Meta情報を取得
	const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
	// 全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel) {
		// MipMapLevelを取得して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		// Textureに転送
		HRESULT hr = texture->WriteToSubresource(
		    UINT(mipLevel),
		    nullptr,              // 全領域へコピー
		    img->pixels,          // 元データアドレス
		    UINT(img->rowPitch),  // 1ラインサイズ
		    UINT(img->slicePitch) // 1枚サイズ
		);
		assert(SUCCEEDED(hr));
	}
}
