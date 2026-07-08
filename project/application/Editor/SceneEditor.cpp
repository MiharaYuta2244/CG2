#include "SceneEditor.h"
#include "Collision.h"
#include "MathUtility.h"
#include "SceneManager.h"
#include <Windows.h>

void SceneEditor::Initialize() {
	selectedGameObject_ = nullptr;
	currentGizmoOperation_ = ImGuizmo::TRANSLATE;
	currentGizmoMode_ = ImGuizmo::LOCAL;
}

void SceneEditor::Update(
    const SceneContext& ctx, const std::vector<IGameObject*>& objects, Player* player, EnemyManager* enemyManager, CameraDeathZoomController* cameraZoomController, float& cameraPosY,
    bool& isDebugCameraActive, Vector3& currentCameraPivot) {
	// 毎フレーム最新のオブジェクトリストを保持
	objects_ = objects;

	UpdatePicking(ctx);
	UpdateImGui(ctx, player, cameraPosY);
	DebugInput(ctx, player, enemyManager, cameraZoomController, isDebugCameraActive, currentCameraPivot);
}

void SceneEditor::UpdateImGui(const SceneContext& ctx, Player* player, float& cameraPosY) {
#ifdef USE_IMGUI
	// 選択中のオブジェクトが破棄されていないか検証
	bool isSelectedValid = false;
	for (IGameObject* obj : objects_) {
		if (obj == selectedGameObject_) {
			isSelectedValid = true;
			break;
		}
	}

	// 存在しなければ選択を解除
	if (!isSelectedValid) {
		selectedGameObject_ = nullptr;
	}

	// =====================================
	// オブジェクトマネージャー
	// =====================================
	ImGui::Begin("Object Manager");

	if (ImGui::RadioButton("Translate", currentGizmoOperation_ == ImGuizmo::TRANSLATE)) {
		currentGizmoOperation_ = ImGuizmo::TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", currentGizmoOperation_ == ImGuizmo::ROTATE)) {
		currentGizmoOperation_ = ImGuizmo::ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", currentGizmoOperation_ == ImGuizmo::SCALE)) {
		currentGizmoOperation_ = ImGuizmo::SCALE;
	}

	ImGui::Separator();

	ImGui::Text("Objects");
	ImGui::BeginChild("ObjectList", ImVec2(0, 150), true);
	for (size_t i = 0; i < objects_.size(); ++i) {
		IGameObject* obj = objects_[i];
		std::string label = obj->GetName() + " ##" + std::to_string(i);

		bool isSelected = (selectedGameObject_ == obj);
		if (ImGui::Selectable(label.c_str(), isSelected)) {
			selectedGameObject_ = obj;
		}
	}
	ImGui::EndChild();
	ImGui::End();

	// =====================================
	// デバッグ用パラメータリスト
	// =====================================
	ImGui::Begin("Debug List");
	if (ImGui::Button("Kill Player")) {
		player->Damage(player->GetMaxHP());
	}
	ImGui::DragFloat("CameraPosY", &cameraPosY, 0.1f);
	ImGui::DragFloat3("Direction", &ctx.engineContext->object3dCommon->GetDirectionalLight().direction.x, 0.01f);
	ImGui::DragFloat3("Color", &ctx.engineContext->object3dCommon->GetDirectionalLight().color.x, 0.01f);
	ImGui::DragFloat3("Intensity", &ctx.engineContext->object3dCommon->GetDirectionalLight().intensity, 0.01f);
	ImGui::End();

	// =====================================
	// ギズモ描画レイヤー
	// =====================================
	if (selectedGameObject_ != nullptr) {
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::Begin("GizmoLayer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar);

		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::Enable(true);
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

		Transform& transform = selectedGameObject_->GetTransform();
		float degRot[3] = {transform.rotate.x * 180.0f / std::numbers::pi_v<float>, transform.rotate.y * 180.0f / std::numbers::pi_v<float>, transform.rotate.z * 180.0f / std::numbers::pi_v<float>};

		float objectMatrix[16];
		ImGuizmo::RecomposeMatrixFromComponents(&transform.translate.x, degRot, &transform.scale.x, objectMatrix);
		Matrix4x4 viewMat = ctx.currentCamera->GetViewMatrix();
		Matrix4x4 projMat = ctx.currentCamera->GetProjection();

		ImGuizmo::Manipulate(&viewMat.m[0][0], &projMat.m[0][0], currentGizmoOperation_, currentGizmoMode_, objectMatrix);

		if (ImGuizmo::IsUsing()) {
			float newTrans[3], newRot[3], newScale[3];
			ImGuizmo::DecomposeMatrixToComponents(objectMatrix, newTrans, newRot, newScale);

			float oldY = transform.translate.y;
			transform.translate = {newTrans[0], newTrans[1], newTrans[2]};
			transform.translate.y = oldY;
			transform.scale = {newScale[0], newScale[1], newScale[2]};
			transform.rotate = {newRot[0] * std::numbers::pi_v<float> / 180.0f, newRot[1] * std::numbers::pi_v<float> / 180.0f, newRot[2] * std::numbers::pi_v<float> / 180.0f};
		}

		ImGui::End();
		ImGui::PopStyleColor();
	}
#endif // USE_IMGUI
}

void SceneEditor::UpdatePicking(const SceneContext& ctx) {
	if (ImGuizmo::IsOver() || ImGuizmo::IsUsing()) {
		return;
	}

	if (ctx.keyboard->MouseButtonTriggered(0)) {
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		auto hwnd = ctx.engineContext->object3dCommon->GetDxCommon()->GetWinApp()->GetHWND();
		ScreenToClient(hwnd, &cursorPos);

		float screenWidth = 1280.0f;
		float screenHeight = 720.0f;
		float nx = (2.0f * cursorPos.x) / screenWidth - 1.0f;
		float ny = 1.0f - (2.0f * cursorPos.y) / screenHeight;

		Matrix4x4 viewMat = ctx.currentCamera->GetViewMatrix();
		Matrix4x4 projMat = ctx.currentCamera->GetProjection();
		Matrix4x4 vpMat = MathUtility::Multiply(viewMat, projMat);
		Matrix4x4 vpInv = MathUtility::Inverse(vpMat);

		Vector3 a = {nx, ny, 0.0f};
		Vector3 b = {nx, ny, 1.0f};
		Vector3 nearPoint = MathUtility::Transform(a, vpInv);
		Vector3 farPoint = MathUtility::Transform(b, vpInv);

		Ray ray;
		ray.origin = nearPoint;
		ray.direction = MathUtility::Normalize(farPoint - nearPoint);

		float minDistance = FLT_MAX;
		IGameObject* hitObject = nullptr;

		for (IGameObject* obj : objects_) {
			AABB aabb = obj->GetAABBForGizmo();
			float distance = 0.0f;

			if (Collision::Intersect(ray, aabb, distance)) {
				if (distance < minDistance) {
					minDistance = distance;
					hitObject = obj;
				}
			}
		}

		if (hitObject) {
			selectedGameObject_ = hitObject;
		} else {
			selectedGameObject_ = nullptr;
		}
	}
}

void SceneEditor::DebugInput(
    const SceneContext& ctx, Player* player, EnemyManager* enemyManager, CameraDeathZoomController* cameraZoomController, bool& isDebugCameraActive, Vector3& currentCameraPivot) {
#if _DEBUG || NDEBUG
	if (ctx.keyboard->KeyTriggered(DIK_F1)) {
		ctx.sceneManager->ChangeScene("EasingEditorScene");
	}

	if (ctx.keyboard->KeyTriggered(DIK_F2)) {
		isDebugCameraActive = !isDebugCameraActive;

		if (isDebugCameraActive) {
			ctx.currentCamera->SetTranslation({0.0f, 200.0f, 0.0f});
		} else {
			ctx.currentCamera->SetTranslation({player->GetPosition().x, 60.0f, player->GetPosition().z});
			ctx.currentCamera->SetPivot(currentCameraPivot);
		}
	}

	if (ctx.keyboard->KeyTriggered(DIK_F3)) {
		enemyManager->SetMove();
	}

	if (ctx.keyboard->KeyTriggered(DIK_F4)) {
		enemyManager->SetStop();
	}

	if (ctx.keyboard->KeyTriggered(DIK_SPACE) && cameraZoomController->GetIsActive()) {
		cameraZoomController->Skip();
	}
#endif
}