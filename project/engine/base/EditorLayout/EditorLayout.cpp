#include "EditorLayout.h"
#ifdef _DEBUG
#include "WinApp.h"
#include "imgui.h"

void EditorLayout::Initialize(SrvManager* srv, DirectXCommon* dx) {
	srv_ = srv;
	dx_ = dx;

	// ImGuiをDockspace対応にする
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void EditorLayout::BeginEditorLayout() {
	// 画面全体をカバーするDockspaceウィンドウ
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
	                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("##EditorRoot", nullptr, hostFlags);
	ImGui::PopStyleVar(3);

	// メニューバーをここに埋め込む
	DrawMainMenuBar();

	// Dockspace生成
	ImGuiID dockId = ImGui::GetID("EditorDockspace");
	ImGui::DockSpace(dockId, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::End();
}

void EditorLayout::EndEditorLayout() {
	// 各パネルはBeginEditorLayout後、Draw系で個別に呼ぶ
}

void EditorLayout::DrawMainMenuBar() {
	if (ImGui::BeginMainMenuBar()) {

		// --- File ---
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit")) { /* 終了フラグを立てる */
			}
			ImGui::EndMenu();
		}

		// --- Edit ---
		if (ImGui::BeginMenu("Edit")) {
			ImGui::MenuItem("Undo", "Ctrl+Z");
			ImGui::MenuItem("Redo", "Ctrl+Y");
			ImGui::EndMenu();
		}

		// --- 中央: Play / Pause ボタン ---
		float menuWidth = ImGui::GetContentRegionAvail().x;
		float btnWidth = 70.0f * 3 + ImGui::GetStyle().ItemSpacing.x * 2;
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - btnWidth) * 0.5f);

		// Play
		ImVec4 playColor = isPlaying_ ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) // 再生中: 緑
		                              : ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, playColor);
		if (ImGui::Button("  >  ", ImVec2(70, 0))) {
			isPlaying_ = !isPlaying_;
		}
		ImGui::PopStyleColor();

		// Pause
		ImGui::SameLine();
		if (ImGui::Button("  ||  ", ImVec2(70, 0))) { /* pause */
		}

		// Step
		ImGui::SameLine();
		if (ImGui::Button("  |>  ", ImVec2(70, 0))) { /* step */
		}

		ImGui::EndMainMenuBar();
	}
}

void EditorLayout::DrawGameView(uint32_t colorSrvIndex) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Game");

	// ウィンドウサイズに合わせてテクスチャをストレッチ表示
	ImVec2 viewSize = ImGui::GetContentRegionAvail();
	if (viewSize.x < 1.0f)
		viewSize.x = 1.0f;
	if (viewSize.y < 1.0f)
		viewSize.y = 1.0f;

	// SRVのGPUハンドルをImGui::Imageに渡す
	// SrvManagerからGPUハンドルを取得してImTextureIDにキャスト
	D3D12_GPU_DESCRIPTOR_HANDLE handle = srv_->GetGPUDescriptorHandle(colorSrvIndex);
	ImTextureID texId = reinterpret_cast<ImTextureID>(handle.ptr);

	ImGui::Image(texId, viewSize);

	// ゲームビュー上にステータスオーバーレイ
	if (!isPlaying_) {
		ImVec2 pos = ImGui::GetItemRectMin();
		pos.x += 10;
		pos.y += 10;
		ImGui::GetWindowDrawList()->AddText(pos, IM_COL32(255, 80, 80, 200), "  EDITOR MODE");
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void EditorLayout::DrawHierarchyPanel() {
	ImGui::Begin("Hierarchy");

	ImGui::Text("Scene");
	ImGui::Separator();

	// サンプル: ダミーオブジェクトリスト
	// 実際はシーンマネージャーからオブジェクト一覧を取得する
	const char* dummyObjects[] = {"Main Camera", "Directional Light", "Player", "Enemy_001", "Enemy_002", "Ground", "SkyBox"};

	for (int i = 0; i < IM_ARRAYSIZE(dummyObjects); ++i) {
		// 選択ハイライト
		bool selected = (selectedObjectId_ == i);
		if (selected) {
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.5f, 0.9f, 0.8f));
		}

		// ツリーノードとして表示 (子なしはLeafフラグ)
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | (selected ? ImGuiTreeNodeFlags_Selected : 0);

		ImGui::TreeNodeEx(dummyObjects[i], flags);
		if (ImGui::IsItemClicked()) {
			selectedObjectId_ = i;
		}

		if (selected)
			ImGui::PopStyleColor();
	}

	// 右クリックメニュー
	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::MenuItem("Create Empty")) { /* 空オブジェクト作成 */
		}
		if (ImGui::MenuItem("Create Camera")) { /* カメラ作成 */
		}
		if (ImGui::MenuItem("Delete")) {
			selectedObjectId_ = -1;
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

void EditorLayout::DrawInspectorPanel() {
	ImGui::Begin("Inspector");

	if (selectedObjectId_ < 0) {
		ImGui::TextDisabled("No object selected.");
		ImGui::End();
		return;
	}

	// --- Transform コンポーネント ---
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		static float pos[3] = {0, 0, 0};
		static float rot[3] = {0, 0, 0};
		static float scale[3] = {1, 1, 1};

		ImGui::DragFloat3("Position", pos, 0.1f);
		ImGui::DragFloat3("Rotation", rot, 0.5f);
		ImGui::DragFloat3("Scale", scale, 0.01f);
	}

	// --- Mesh Renderer コンポーネント (ダミー) ---
	if (ImGui::CollapsingHeader("Mesh Renderer")) {
		ImGui::Text("Mesh: Cube");
		static bool castShadow = true;
		ImGui::Checkbox("Cast Shadow", &castShadow);
		static ImVec4 albedo = {1, 1, 1, 1};
		ImGui::ColorEdit4("Albedo", (float*)&albedo);
	}

	// --- Add Component ボタン ---
	ImGui::Separator();
	float btnW = ImGui::GetContentRegionAvail().x;
	if (ImGui::Button("Add Component", ImVec2(btnW, 0))) {
		ImGui::OpenPopup("AddComponentPopup");
	}
	if (ImGui::BeginPopup("AddComponentPopup")) {
		if (ImGui::MenuItem("Camera")) {
		}
		if (ImGui::MenuItem("Light")) {
		}
		if (ImGui::MenuItem("Particle System")) {
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

void EditorLayout::DrawProjectPanel() {
	ImGui::Begin("Project");

	ImGui::Text("Assets");
	ImGui::Separator();

	// ダミーフォルダ/ファイルリスト
	static const char* assets[] = {"Textures/", "Models/", "Shaders/", "Scenes/GameScene.scene", "Materials/Player.mat"};

	for (const char* asset : assets) {
		ImGui::Selectable(asset);
	}

	ImGui::End();
}
#endif