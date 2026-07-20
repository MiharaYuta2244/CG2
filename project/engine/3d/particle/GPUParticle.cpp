#include "GPUParticle.h"
#include "DirectXUtils.h"
#include "MathUtility.h"
#include <cassert>

using namespace Microsoft::WRL;

GPUParticle::~GPUParticle() {
	if (ctx_ && ctx_->srvManager) {
		ctx_->srvManager->Free(particleUavIndex_);
		ctx_->srvManager->Free(particleSrvIndex_);
		ctx_->srvManager->Free(freeListIndexUavIndex_);
		ctx_->srvManager->Free(freeListUavIndex_);
	}
}

void GPUParticle::Initialize(EngineContext* ctx, const std::string& texturepath) {
	ctx_ = ctx;
	dxCommon_ = ctx->particleCommon->GetDxCommon();
	texturePath_ = texturepath;
	textureFullPath_ = "resources/textures/" + texturepath;

	InitializeShaderCompiler();
	CreateComputePipeline();
	CreateEmitComputePipeline();
	CreateUpdateComputePipeline();
	CreateGraphicsPipeline();
	CreateResources();

	// エミッター初期化
	emitterSphere_.count = 10;
	emitterSphere_.frequency = 0.5f;
	emitterSphere_.frequencyTime = 0.0f;
	emitterSphere_.translate = Vector3(0.0f, 0.0f, 0.0f);
	emitterSphere_.radius = 1.0f;
	emitterSphere_.emit = 0;
	emitterSphere_.seed = 0;
}

void GPUParticle::Update(const PerView& viewData, float deltaTime) {
	// エミッタータイム加算
	emitterSphere_.frequencyTime += deltaTime;
	perFrame_.time += deltaTime;
	perFrame_.deltaTime = deltaTime;

	// 射出間隔を上回ったら射出許可を出して時間を調整
	if (emitterSphere_.frequency <= emitterSphere_.frequencyTime) {
		emitterSphere_.frequencyTime -= emitterSphere_.frequency;
		emitterSphere_.emit = 1;
		++emitterSphere_.seed;
	} else {
		// 射出許可を出さない
		emitterSphere_.emit = 0;
	}

	if (mappedPerView_) {
		*mappedPerView_ = viewData;
	}

	if (mappedEmitter_) {
		*mappedEmitter_ = emitterSphere_;
	}

	if (mappedPerFrame_) {
		mappedPerFrame_->time = perFrame_.time;
		mappedPerFrame_->deltaTime = deltaTime;
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

	ID3D12DescriptorHeap* ppHeaps[] = {dxCommon_->GetSrvDescriptorHeap().Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	commandList->SetGraphicsRootConstantBufferView(0, perViewBuffer_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, particleSRVHeapHandle);
	commandList->SetGraphicsRootConstantBufferView(2, materialBuffer_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(3, ctx_->textureManager->GetSrvHandleGPU(textureFullPath_));
	commandList->DrawInstanced(6, kMaxParticles, 0, 0);
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
	uavRange[0].NumDescriptors = 3;
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
	// VS用 t0 (Particle StructuredBuffer)
	D3D12_DESCRIPTOR_RANGE srvRangeVS[1] = {};
	srvRangeVS[0].BaseShaderRegister = 0;
	srvRangeVS[0].NumDescriptors = 1;
	srvRangeVS[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvRangeVS[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// PS用 t0 (Texture2D)
	D3D12_DESCRIPTOR_RANGE srvRangePS[1] = {};
	srvRangePS[0].BaseShaderRegister = 0;
	srvRangePS[0].NumDescriptors = 1;
	srvRangePS[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvRangePS[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[4] = {};

	// [0] VS用 b0: PerView
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// [1] VS用 t0: Particle StructuredBuffer
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].DescriptorTable.pDescriptorRanges = srvRangeVS;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(srvRangeVS);

	// [2] PS用 b0: Material
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].Descriptor.ShaderRegister = 0;

	// [3] PS用 t0: Texture
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].DescriptorTable.pDescriptorRanges = srvRangePS;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = _countof(srvRangePS);

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0; // s0
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pStaticSamplers = staticSamplers;
	rootSignatureDesc.NumStaticSamplers = _countof(staticSamplers);

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&graphicsRootSignature_));
	assert(SUCCEEDED(hr));

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = nullptr;
	inputLayoutDesc.NumElements = 0;

	// ShaderコンパイルとPipelineStateの作成
	IDxcBlob* vertexShaderBlob = DirectXUtils::CompileShader(L"resources/shaders/GPUParticle.VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	IDxcBlob* pixelShaderBlob = DirectXUtils::CompileShader(L"resources/shaders/Particle.PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	auto& rt = blendDesc.RenderTarget[0];
	rt.BlendEnable = TRUE;
	rt.LogicOpEnable = FALSE;

	// 加算合成
	rt.SrcBlend = D3D12_BLEND_ONE;
	rt.DestBlend = D3D12_BLEND_ONE;
	rt.BlendOp = D3D12_BLEND_OP_ADD;
	rt.SrcBlendAlpha = D3D12_BLEND_ONE;
	rt.DestBlendAlpha = D3D12_BLEND_ONE;
	rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

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
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;

	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}

void GPUParticle::CreateResources() {
	// UAVを連続したインデックスとして確保する (u0, u1, u2 用)
	particleUavIndex_ = ctx_->srvManager->Allocate();
	freeListIndexUavIndex_ = ctx_->srvManager->Allocate();
	freeListUavIndex_ = ctx_->srvManager->Allocate();
	particleSrvIndex_ = ctx_->srvManager->Allocate();

	auto dxCommon = dxCommon_;
	auto device = dxCommon_->GetDevice();
	HRESULT hr;

	// === パーティクル用StructuredBufferの作成 ===
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
	particleDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	particleState_ = D3D12_RESOURCE_STATE_COMMON;
	hr = device->CreateCommittedResource(&heapPropsDefault, D3D12_HEAP_FLAG_NONE, &particleDesc, particleState_, nullptr, IID_PPV_ARGS(&particleBuffer_));
	assert(SUCCEEDED(hr));

	// === PerView用定数バッファの作成 ===
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

	// === Particle UAV / SRV の作成 ===
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = kMaxParticles;
	uavDesc.Buffer.StructureByteStride = sizeof(ParticleCS);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = kMaxParticles;
	srvDesc.Buffer.StructureByteStride = sizeof(ParticleCS);

	D3D12_CPU_DESCRIPTOR_HANDLE uavCpuHandle = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), particleUavIndex_);
	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), particleSrvIndex_);

	device->CreateUnorderedAccessView(particleBuffer_.Get(), nullptr, &uavDesc, uavCpuHandle);
	device->CreateShaderResourceView(particleBuffer_.Get(), &srvDesc, srvCpuHandle);

	particleUAVHeapHandle = dxCommon->GetGPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), particleUavIndex_);
	particleSRVHeapHandle = dxCommon->GetGPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), particleSrvIndex_);

	// === gFreeListIndex 用バッファの作成 (u1) ===
	D3D12_RESOURCE_DESC freeListIndexDesc{};
	freeListIndexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	freeListIndexDesc.Width = sizeof(int);
	freeListIndexDesc.Height = 1;
	freeListIndexDesc.DepthOrArraySize = 1;
	freeListIndexDesc.MipLevels = 1;
	freeListIndexDesc.Format = DXGI_FORMAT_UNKNOWN;
	freeListIndexDesc.SampleDesc.Count = 1;
	freeListIndexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	freeListIndexDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	hr = device->CreateCommittedResource(&heapPropsDefault, D3D12_HEAP_FLAG_NONE, &freeListIndexDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&freeListIndexBuffer_));
	assert(SUCCEEDED(hr));

	D3D12_UNORDERED_ACCESS_VIEW_DESC freeListIndexUavDesc{};
	freeListIndexUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	freeListIndexUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	freeListIndexUavDesc.Buffer.NumElements = 1;
	freeListIndexUavDesc.Buffer.StructureByteStride = sizeof(int);

	D3D12_CPU_DESCRIPTOR_HANDLE freeListIndexUavCpuHandle = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), freeListIndexUavIndex_);
	device->CreateUnorderedAccessView(freeListIndexBuffer_.Get(), nullptr, &freeListIndexUavDesc, freeListIndexUavCpuHandle);

	// === gFreeList 用バッファの作成 (u2) ===
	D3D12_RESOURCE_DESC freeListDesc = freeListIndexDesc;
	freeListDesc.Width = sizeof(int) * kMaxParticles; // kMaxParticles分確保

	hr = device->CreateCommittedResource(&heapPropsDefault, D3D12_HEAP_FLAG_NONE, &freeListDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&freeListBuffer_));
	assert(SUCCEEDED(hr));

	D3D12_UNORDERED_ACCESS_VIEW_DESC freeListUavDesc{};
	freeListUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	freeListUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	freeListUavDesc.Buffer.NumElements = kMaxParticles;
	freeListUavDesc.Buffer.StructureByteStride = sizeof(int);

	D3D12_CPU_DESCRIPTOR_HANDLE freeListUavCpuHandle = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), freeListUavIndex_);
	device->CreateUnorderedAccessView(freeListBuffer_.Get(), nullptr, &freeListUavDesc, freeListUavCpuHandle);

	// === Material 用定数バッファ作成 ===
	uint32_t materialBufferSize = (sizeof(ParticleMaterial) + 255) & ~255;
	D3D12_RESOURCE_DESC materialDesc{};
	materialDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	materialDesc.Width = materialBufferSize;
	materialDesc.Height = 1;
	materialDesc.DepthOrArraySize = 1;
	materialDesc.MipLevels = 1;
	materialDesc.SampleDesc.Count = 1;
	materialDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	hr = device->CreateCommittedResource(&heapPropsUpload, D3D12_HEAP_FLAG_NONE, &materialDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&materialBuffer_));
	assert(SUCCEEDED(hr));
	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedMaterial_));

	// === Emitter 用定数バッファ作成 ===
	uint32_t emitterBufferSize = (sizeof(EmitterSphere) + 255) & ~255;
	D3D12_RESOURCE_DESC emitterDesc{};
	emitterDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	emitterDesc.Width = emitterBufferSize;
	emitterDesc.Height = 1;
	emitterDesc.DepthOrArraySize = 1;
	emitterDesc.MipLevels = 1;
	emitterDesc.Format = DXGI_FORMAT_UNKNOWN;
	emitterDesc.SampleDesc.Count = 1;
	emitterDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	hr = device->CreateCommittedResource(&heapPropsUpload, D3D12_HEAP_FLAG_NONE, &emitterDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&emitterBuffer_));
	assert(SUCCEEDED(hr));
	emitterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedEmitter_));
	*mappedEmitter_ = emitterSphere_;

	// === PerFrame 用定数バッファ作成 ===
	uint32_t perFrameSize = (sizeof(PerFrame) + 255) & ~255;
	D3D12_RESOURCE_DESC perFrameDesc{};
	perFrameDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	perFrameDesc.Width = perFrameSize;
	perFrameDesc.Height = 1;
	perFrameDesc.DepthOrArraySize = 1;
	perFrameDesc.MipLevels = 1;
	perFrameDesc.Format = DXGI_FORMAT_UNKNOWN;
	perFrameDesc.SampleDesc.Count = 1;
	perFrameDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(&heapPropsUpload, D3D12_HEAP_FLAG_NONE, &perFrameDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&perFrameBuffer_));
	perFrameBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedPerFrame_));

	// デフォルト値
	mappedMaterial_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	mappedMaterial_->uvTransform = MathUtility::MakeIdentity4x4();
	mappedMaterial_->alphaCutoff = 0.0f;
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

	// 共有SRV/UAVヒープをセット
	ID3D12DescriptorHeap* ppHeaps[] = {dxCommon_->GetSrvDescriptorHeap().Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	commandList->SetComputeRootDescriptorTable(0, particleUAVHeapHandle);
	commandList->Dispatch(1, 1, 1);
}

void GPUParticle::CreateEmitComputePipeline() {
	HRESULT hr;

	D3D12_DESCRIPTOR_RANGE uavRange[1] = {};
	uavRange[0].BaseShaderRegister = 0;
	uavRange[0].NumDescriptors = 3;
	uavRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// [0] b0: EmitterSphere (CBV)
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// [1] b1: PerFrame
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 1;

	// [2] u0: Particle StructuredBuffer
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = uavRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(uavRange);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumParameters = _countof(rootParameters);

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&emitRootSignature_));
	assert(SUCCEEDED(hr));

	IDxcBlob* emitShaderBlob = DirectXUtils::CompileShader(L"resources/shaders/EmitParticle.CS.hlsl", L"cs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(emitShaderBlob != nullptr);

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
	computePipelineDesc.pRootSignature = emitRootSignature_.Get();
	computePipelineDesc.CS = {emitShaderBlob->GetBufferPointer(), emitShaderBlob->GetBufferSize()};

	hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&emitPipelineState_));
	assert(SUCCEEDED(hr));
}

void GPUParticle::DispatchEmit() {
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

	commandList->SetComputeRootSignature(emitRootSignature_.Get());
	commandList->SetPipelineState(emitPipelineState_.Get());

	ID3D12DescriptorHeap* ppHeaps[] = {dxCommon_->GetSrvDescriptorHeap().Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	commandList->SetComputeRootConstantBufferView(0, emitterBuffer_->GetGPUVirtualAddress());
	commandList->SetComputeRootConstantBufferView(1, perFrameBuffer_->GetGPUVirtualAddress());
	commandList->SetComputeRootDescriptorTable(2, particleUAVHeapHandle);

	commandList->Dispatch(1, 1, 1);
}

void GPUParticle::CreateUpdateComputePipeline() {
	HRESULT hr;

	// RootSignature: b1 (PerFrame), u0 (Particles)
	D3D12_DESCRIPTOR_RANGE uavRange[1] = {};
	uavRange[0].BaseShaderRegister = 0;
	uavRange[0].NumDescriptors = 3;
	uavRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2] = {};

	// b1: PerFrame
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].Descriptor.ShaderRegister = 1;

	// u0: Particle StructuredBuffer
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].DescriptorTable.pDescriptorRanges = uavRange;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(uavRange);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumParameters = _countof(rootParameters);

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&updateRootSignature_));
	assert(SUCCEEDED(hr));

	// Shader compile
	IDxcBlob* updateShaderBlob = DirectXUtils::CompileShader(L"resources/shaders/UpdateParticle.CS.hlsl", L"cs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(updateShaderBlob != nullptr);

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
	computePipelineDesc.pRootSignature = updateRootSignature_.Get();
	computePipelineDesc.CS = {updateShaderBlob->GetBufferPointer(), updateShaderBlob->GetBufferSize()};

	hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&updatePipelineState_));
	assert(SUCCEEDED(hr));
}

void GPUParticle::DispatchUpdate() {
	auto commandList = dxCommon_->GetCommandList();

	// 事前にバッファの状態をUAVに遷移させるか、既にUAVならUAVバリアを張る
	if (particleState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleBuffer_.Get();
		barrier.Transition.StateBefore = particleState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrier);
		particleState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	} else {
		// 連続してUAVとして使用するため、UAVバリアを張る
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = particleBuffer_.Get();
		commandList->ResourceBarrier(1, &barrier);
	}

	commandList->SetComputeRootSignature(updateRootSignature_.Get());
	commandList->SetPipelineState(updatePipelineState_.Get());

	ID3D12DescriptorHeap* ppHeaps[] = {dxCommon_->GetSrvDescriptorHeap().Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	commandList->SetComputeRootConstantBufferView(0, perFrameBuffer_->GetGPUVirtualAddress());
	commandList->SetComputeRootDescriptorTable(1, particleUAVHeapHandle);
	commandList->Dispatch(kMaxParticles, 1, 1);
}