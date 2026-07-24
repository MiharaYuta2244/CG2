#pragma once
#include <wrl.h>
#include "Material.h"
#include "Model.h"
#include "Transform.h"
#include "Vector4.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>

class SrvManager;

/// <summary>
/// ImGuiの管理を行うクラス
/// </summary>
class ImGuiManager {
public:
	void
	    Initialize(HWND hwnd, Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, UINT numFramesInFlight, DXGI_FORMAT format, SrvManager* srvManager);
	void BeginFrame();
	void BeginDockSpace();
	void EndDockSpace();
	void Render(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
	void Finalize();
	void SetModel(const std::shared_ptr<Model>& model);

private:
	std::shared_ptr<Model> model_;
};
