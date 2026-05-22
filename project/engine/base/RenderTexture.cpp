#include "RenderTexture.h"

void RenderTexture::Initialize(DirectXCommon* dx, SrvManager* srv, uint32_t width, uint32_t height) {
	auto device = dx->GetDevice();

	resourceColor_ = dx->CreateRenderTextureResource(dx->GetDevice(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.2f, 0.2f, 0.2f, 1.0f});

	resourceDepth_ = dx->CreateDepthStencilTextureResource(device, width, height);

	// RTVを作成
	rtvHandle_ = dx->CreateRTV(resourceColor_.Get());

	// DSVを作成
	dsvHandle_ = dx->CreateDSV(resourceDepth_.Get());

	// SRVを作成
	srvIndexColor_ = srv->Allocate();
	srv->CreateSRVforTexture2D(srvIndexColor_, resourceColor_.Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);
}

void RenderTexture::BeginRender(DirectXCommon* dx) {
	auto cmd = dx->GetCommandList();

	// Color を RenderTarget に遷移
	D3D12_RESOURCE_BARRIER barrierColor{};
	barrierColor.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierColor.Transition.pResource = resourceColor_.Get();
	barrierColor.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrierColor.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	cmd->ResourceBarrier(1, &barrierColor);

	// Depth を DepthWrite に遷移
	D3D12_RESOURCE_BARRIER barrierDepth{};
	barrierDepth.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDepth.Transition.pResource = resourceDepth_.Get();
	barrierDepth.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_READ;
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

	// Color を ShaderResource に戻す
	D3D12_RESOURCE_BARRIER barrierColor{};
	barrierColor.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierColor.Transition.pResource = resourceColor_.Get();
	barrierColor.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierColor.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	cmd->ResourceBarrier(1, &barrierColor);

	// Depth を DepthRead に戻す
	D3D12_RESOURCE_BARRIER barrierDepth{};
	barrierDepth.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDepth.Transition.pResource = resourceDepth_.Get();
	barrierDepth.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrierDepth.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_READ;
	cmd->ResourceBarrier(1, &barrierDepth);
}
