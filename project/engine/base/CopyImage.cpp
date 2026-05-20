#include "CopyImage.h"
#include "DirectXUtils.h"
#include <assert.h>
#include <d3d12.h>

using namespace Microsoft::WRL;
using namespace TinyEngine;

void CopyImage::InitializeShaderCompiler() {
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

void CopyImage::CreateGraphicsPipeline(DirectXCommon* dx) {
	auto device = dx->GetDevice();
	assert(device);

	// シェーダーコンパイル
	IDxcBlob* vsBlob = DirectXUtils::CompileShader(L"resources/shaders/FullScreen.VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(vsBlob != nullptr);
	IDxcBlob* psBlob = DirectXUtils::CompileShader(L"resources/shaders/" + shaderMap_.at(postEffectType_) + L".PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(psBlob != nullptr);

	// ルートシグネチャを作成
	D3D12_DESCRIPTOR_RANGE descriptorRange1 = {};
	descriptorRange1.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange1.NumDescriptors = 1;
	descriptorRange1.BaseShaderRegister = 0;
	descriptorRange1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange1;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	D3D12_STATIC_SAMPLER_DESC staticSampler = {};
	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = 0;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootDesc.pParameters = rootParameters;
	rootDesc.NumParameters = _countof(rootParameters);
	rootDesc.pStaticSamplers = &staticSampler;
	rootDesc.NumStaticSamplers = 1;

	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
	if (signatureBlob) signatureBlob->Release();
	if (errorBlob) errorBlob->Release();

	// PSO設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	ZeroMemory(&psoDesc, sizeof(psoDesc));
	psoDesc.pRootSignature = rootSignature_.Get();
	psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
	psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};

	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.DepthClipEnable = true;

	psoDesc.BlendState.AlphaToCoverageEnable = false;
	psoDesc.BlendState.IndependentBlendEnable = false;
	for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
		psoDesc.BlendState.RenderTarget[i].BlendEnable = false;
		psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.StencilEnable = false;

	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = dx->GetRtvFormat();
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	psoDesc.SampleDesc.Count = 1;

	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
	assert(SUCCEEDED(hr));
}

void TinyEngine::CopyImage::CreateVignetteCB() {
	// ヴィネット用のリソース作成
	vignetteCB_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(VignetteParam));
	// 書き込むためのアドレスを取得
	vignetteCB_->Map(0, nullptr, reinterpret_cast<void**>(&vignetteData_));
	vignetteCB_->Unmap(0, nullptr);
	// 書き込み
	*vignetteData_ = vignetteParam_;
}

void CopyImage::Initialize(DirectXCommon* dx, PostEffectType type) {
	dxCommon_ = dx;

	// 適用するポストエフェクトのタイプを設定
	postEffectType_ = type;

	// リソース作成
	CreateVignetteCB();

	// シェーダコンパイラ初期化
	InitializeShaderCompiler();

	// PSO,RootSignature作成
	CreateGraphicsPipeline(dx);
}

void CopyImage::Draw(DirectXCommon* dx, SrvManager* srv, uint32_t srvIndex) {
	auto cmd = dx->GetCommandList();
	assert(cmd);

	// ルートシグネチャとPSOをセット
	cmd->SetGraphicsRootSignature(rootSignature_.Get());
	cmd->SetPipelineState(pipelineState_.Get());

	// SRVヒープをセット
	ID3D12DescriptorHeap* descriptorHeaps[] = {dx->GetSrvDescriptorHeap().Get()};
	cmd->SetDescriptorHeaps(1, descriptorHeaps);

	// ルートテーブルにSRVをセット
	cmd->SetGraphicsRootDescriptorTable(0, srv->GetGPUDescriptorHandle(srvIndex));

	// CBV
	cmd->SetGraphicsRootConstantBufferView(1, vignetteCB_->GetGPUVirtualAddress());

	// IA
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 描画
	cmd->DrawInstanced(3, 1, 0, 0);
}