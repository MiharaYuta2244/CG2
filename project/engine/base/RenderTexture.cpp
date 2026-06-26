#include "RenderTexture.h"

RenderTexture::~RenderTexture(){
	// RTV/DSVインデックスの解放
	if (dxCommon_) {
		dxCommon_->FreeRTV(rtvIndex_);
		dxCommon_->FreeDSV(dsvIndex_);
	}
	// SRVインデックスの解放
	if (srvManager_) {
		srvManager_->Free(srvIndexColor_);
		srvManager_->Free(srvIndexDepth_);
	}

	Logger::Log("RenderTexture Destructor Called!\n", LogLevel::Error);
}

void RenderTexture::Initialize(DirectXCommon* dx, SrvManager* srv, uint32_t width, uint32_t height) {
	dxCommon_ = dx;
	srvManager_ = srv;

	auto device = dx->GetDevice();

	resourceColor_ = dx->CreateRenderTextureResource(dx->GetDevice(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.2f, 0.2f, 0.2f, 1.0f});
	resourceDepth_ = dx->CreateDepthStencilTextureResource(device, width, height);

	// RTVのインデックスを確保し、それを使って作成・ハンドル取得を行う
	rtvIndex_ = dx->AllocateRTV();
	dx->CreateRTV(resourceColor_.Get(), rtvIndex_);
	rtvHandle_ = dx->GetRTVHandle(rtvIndex_);

	// DSVのインデックスを確保し、それを使って作成・ハンドル取得を行う
	dsvIndex_ = dx->AllocateDSV();
	dx->CreateDSV(resourceDepth_.Get(), dsvIndex_);
	dsvHandle_ = dx->GetDSVHandle(dsvIndex_);

	// SRVを作成
	srvIndexColor_ = srv->Allocate();
	srv->CreateSRVforTexture2D(srvIndexColor_, resourceColor_.Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);

	srvIndexDepth_ = srv->Allocate();
	srv->CreateSRVforTexture2D(srvIndexDepth_, resourceDepth_.Get(), DXGI_FORMAT_R24_UNORM_X8_TYPELESS, 1);

	D3D12_RESOURCE_BARRIER initBarrier{};
	initBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	initBarrier.Transition.pResource = resourceDepth_.Get();
	initBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_READ;
	initBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	dx->GetCommandList()->ResourceBarrier(1, &initBarrier);
}

void RenderTexture::BeginRender(DirectXCommon* dx) {
	auto cmd = dx->GetCommandList();

	// Color
	D3D12_RESOURCE_BARRIER barrierColor{};
	barrierColor.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierColor.Transition.pResource = resourceColor_.Get();
	barrierColor.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrierColor.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	cmd->ResourceBarrier(1, &barrierColor);

	// Depth
	D3D12_RESOURCE_BARRIER barrierDepth{};
	barrierDepth.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDepth.Transition.pResource = resourceDepth_.Get();
	barrierDepth.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrierDepth.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	cmd->ResourceBarrier(1, &barrierDepth);

	// RTV + DSV をセット
	cmd->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle_);

	// Color Clear
	float clearColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};
	cmd->ClearRenderTargetView(rtvHandle_, clearColor, 0, nullptr);

	// Depth Clear
	cmd->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	viewport_ = dx->CreateViewport();
	scissorRect_ = dx->CreateScissor();

	cmd->RSSetViewports(1, &viewport_);       // Viewportを設定
	cmd->RSSetScissorRects(1, &scissorRect_); // Scissorを設定
}

void RenderTexture::EndRender(DirectXCommon* dx) {
	auto cmd = dx->GetCommandList();

	// Color
	D3D12_RESOURCE_BARRIER barrierColor{};
	barrierColor.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierColor.Transition.pResource = resourceColor_.Get();
	barrierColor.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierColor.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	cmd->ResourceBarrier(1, &barrierColor);

	// Depth
	D3D12_RESOURCE_BARRIER barrierDepth{};
	barrierDepth.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDepth.Transition.pResource = resourceDepth_.Get();
	barrierDepth.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrierDepth.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	cmd->ResourceBarrier(1, &barrierDepth);
}
