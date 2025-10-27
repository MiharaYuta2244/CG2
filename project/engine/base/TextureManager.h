#pragma once
#include "DirectXTex.h"
#include "MaterialData.h"
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include "TextureData.h"

class DirectXCommon;

class TextureManager {
public:
	~TextureManager();
	void Initialize(DirectXCommon* directXCommon);
	void LoadTexture(const std::string& filePath);

	// SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	// テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

	// メタデータを取得
	const DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);

	/*D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU() { return textureSrvHandleGPU_; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU2() { return textureSrvHandleGPU2_; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU3() { return textureSrvHandleGPU3_; }*/

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	void UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

private:
	// テクスチャデータ
	std::vector<TextureData> textureDatas_;
	DirectXCommon* directXCommon_;

	// SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;

	/*Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;
	DirectX::ScratchImage mipImage_;
	DirectX::ScratchImage mipImage2_;
	DirectX::ScratchImage mipImage3_;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2_;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource3_;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2_;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU3_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU3_;*/
};
