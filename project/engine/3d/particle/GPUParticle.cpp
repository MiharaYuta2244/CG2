#include "GPUParticle.h"
#include "DirectXUtils.h"
#include <cassert>

using namespace Microsoft::WRL;

void GPUParticle::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	InitializeShaderCompiler();
	CreateComputePipeline();
	CreateGraphicsPipeline();
	CreateResources();
}

void GPUParticle::Update(const PerView& viewData) {
	if (mappedPerView_) {
		*mappedPerView_ = viewData;
	}
}

void GPUParticle::Draw() {
	auto commandList = dxCommon_->GetCommandList();

	// 描画前にバッファの状態をSRVに遷移させる
	if (particleState_ != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleBuffer_.Get();
		barrier.Transition.StateBefore = particleState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrier);
		particleState_ = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}

	commandList->SetGraphicsRootSignature(graphicsRootSignature_.Get());     
	commandList->SetPipelineState(graphicsPipelineState_.Get());             
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// デスクリプタヒープをコマンドリストにセット
	ID3D12DescriptorHeap* ppHeaps[] = {srvUavHeap_.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	commandList->SetGraphicsRootConstantBufferView(0, perViewBuffer_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, particleSRVHeapHandle);                    

	commandList->DrawInstanced(4, kMaxParticles, 0, 0);
}

void GPUParticle::InitializeShaderCompiler() {
	HRESULT hr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

void GPUParticle::CreateComputePipeline() {
	HRESULT hr;

	// RootSignatureの作成
	D3D12_DESCRIPTOR_RANGE uavRange[1] = {};
	uavRange[0].BaseShaderRegister = 0;
	uavRange[0].NumDescriptors = 1;
	uavRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = uavRange;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(uavRange);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumParameters = _countof(rootParameters);

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&computeRootSignature_));
	assert(SUCCEEDED(hr));

	// ComputeShaderのコンパイルとPipelineStateの作成
	IDxcBlob* computeShaderBlob = DirectXUtils::CompileShader(L"resources/shaders/InitializeParticle.CS.hlsl", L"cs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(computeShaderBlob != nullptr);

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
	computePipelineDesc.pRootSignature = computeRootSignature_.Get();
	computePipelineDesc.CS = {computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize()};

	hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&computePipelineState_));
	assert(SUCCEEDED(hr));
}

void GPUParticle::CreateGraphicsPipeline() {
	HRESULT hr;

	// RootSignatureの作成
	D3D12_DESCRIPTOR_RANGE srvRange[1] = {};
	srvRange[0].BaseShaderRegister = 0;
	srvRange[0].NumDescriptors = 1;
	srvRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	// b0: PerView
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// t0: Particle StructuredBuffer
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].DescriptorTable.pDescriptorRanges = srvRange;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(srvRange);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumParameters = _countof(rootParameters);

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&graphicsRootSignature_));
	assert(SUCCEEDED(hr));

	// InputLayoutの設定
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "COLOR";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// ShaderコンパイルとPipelineStateの作成
	IDxcBlob* vertexShaderBlob = DirectXUtils::CompileShader(L"resources/shaders/GPUParticle.VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	IDxcBlob* pixelShaderBlob = DirectXUtils::CompileShader(L"resources/shaders/Particle.PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = graphicsRootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = {vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()};
	graphicsPipelineStateDesc.PS = {pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()};
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = dxCommon_->GetRtvFormat();
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}

void GPUParticle::CreateResources() {
	auto device = dxCommon_->GetDevice();
	HRESULT hr;

	// パーティクル用StructuredBufferの作成
	uint32_t particleBufferSize = sizeof(ParticleCS) * kMaxParticles;

	D3D12_HEAP_PROPERTIES heapPropsDefault{};
	heapPropsDefault.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC particleDesc{};
	particleDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	particleDesc.Width = particleBufferSize;
	particleDesc.Height = 1;
	particleDesc.DepthOrArraySize = 1;
	particleDesc.MipLevels = 1;
	particleDesc.Format = DXGI_FORMAT_UNKNOWN;
	particleDesc.SampleDesc.Count = 1;
	particleDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	particleDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // UAV用フラグ必須

	// 初期状態はCOMMONにしておく
	particleState_ = D3D12_RESOURCE_STATE_COMMON;
	hr = device->CreateCommittedResource(&heapPropsDefault, D3D12_HEAP_FLAG_NONE, &particleDesc, particleState_, nullptr, IID_PPV_ARGS(&particleBuffer_));
	assert(SUCCEEDED(hr));

	// PerView用定数バッファの作成
	uint32_t perViewBufferSize = (sizeof(PerView) + 255) & ~255;
	D3D12_HEAP_PROPERTIES heapPropsUpload{};
	heapPropsUpload.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC perViewDesc{};
	perViewDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	perViewDesc.Width = perViewBufferSize;
	perViewDesc.Height = 1;
	perViewDesc.DepthOrArraySize = 1;
	perViewDesc.MipLevels = 1;
	perViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	perViewDesc.SampleDesc.Count = 1;
	perViewDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	hr = device->CreateCommittedResource(&heapPropsUpload, D3D12_HEAP_FLAG_NONE, &perViewDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&perViewBuffer_));
	assert(SUCCEEDED(hr));
	perViewBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedPerView_));

	// SRVとUAVを格納するDescriptorHeapの作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NumDescriptors = 2; // UAV用とSRV用の2つ
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // シェーダーからアクセス可能にする
	hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvUavHeap_));
	assert(SUCCEEDED(hr));

	// Viewの作成
	uint32_t incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = srvUavHeap_->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srvUavHeap_->GetGPUDescriptorHandleForHeapStart();

	// UAVの作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = kMaxParticles;
	uavDesc.Buffer.StructureByteStride = sizeof(ParticleCS);
	device->CreateUnorderedAccessView(particleBuffer_.Get(), nullptr, &uavDesc, cpuHandle);

	particleUAVHeapHandle = gpuHandle;

	// ハンドルを進める
	cpuHandle.ptr += incrementSize;
	gpuHandle.ptr += incrementSize;

	// SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = kMaxParticles;
	srvDesc.Buffer.StructureByteStride = sizeof(ParticleCS);
	device->CreateShaderResourceView(particleBuffer_.Get(), &srvDesc, cpuHandle);

	particleSRVHeapHandle = gpuHandle;
}

void GPUParticle::DispatchInitialize() {
	auto commandList = dxCommon_->GetCommandList();

	// 事前にバッファの状態をUAVに遷移させる
	if (particleState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleBuffer_.Get();
		barrier.Transition.StateBefore = particleState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrier);
		particleState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	commandList->SetComputeRootSignature(computeRootSignature_.Get());
	commandList->SetPipelineState(computePipelineState_.Get());       

	// デスクリプタヒープをコマンドリストにセット
	ID3D12DescriptorHeap* ppHeaps[] = {srvUavHeap_.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	commandList->SetComputeRootDescriptorTable(0, particleUAVHeapHandle);
	commandList->Dispatch(1, 1, 1);
}