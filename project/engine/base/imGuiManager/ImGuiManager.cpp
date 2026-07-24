#include "ImGuiManager.h"
#include "WinApp.h"
#include "SrvManager.h"
#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

using namespace Microsoft::WRL;

void ImGuiManager::SetModel(const std::shared_ptr<Model>& model) {
	model_ = model; // 同じインスタンスを共有 {
}

void ImGuiManager::Initialize(HWND hwnd, ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> commandQueue, UINT numFramesInFlight, DXGI_FORMAT format, SrvManager* srvManager) {
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = device.Get();
	initInfo.CommandQueue = commandQueue.Get();
	initInfo.NumFramesInFlight = numFramesInFlight;
	initInfo.RTVFormat = format;
	initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
	initInfo.UserData = srvManager;

	// テクスチャ用SRVディスクリプタの確保/解放をSrvManagerに委譲する
	initInfo.SrvDescriptorHeap = srvManager->GetDescriptorHeap().Get();
	initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle) {
		SrvManager* srv = static_cast<SrvManager*>(info->UserData);
		uint32_t index = srv->Allocate();
		*outCpuHandle = srv->GetCPUDescriptorHandle(index);
		*outGpuHandle = srv->GetGPUDescriptorHandle(index);
	};
	initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE) {
		SrvManager* srv = static_cast<SrvManager*>(info->UserData);
		srv->Free(srv->GetIndexFromHandle(cpuHandle));
	};

	ImGui_ImplDX12_Init(&initInfo);
#endif
}

void ImGuiManager::BeginFrame() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
}

void ImGuiManager::Render(ComPtr<ID3D12GraphicsCommandList> commandList) {
#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
#endif
}

void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

void ImGuiManager::BeginDockSpace() {
#ifdef USE_IMGUI
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
	                             ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpaceHost", nullptr, hostFlags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspaceId = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
#endif
}

void ImGuiManager::EndDockSpace() {
#ifdef USE_IMGUI
	ImGui::End(); // BeginDockSpaceで開いた"DockSpaceHost"を閉じる
#endif
}