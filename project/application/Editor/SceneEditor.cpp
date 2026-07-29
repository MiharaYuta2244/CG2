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

	// 最初のオブジェクトを選択
	if (selectedGameObject_ == nullptr && !objects_.empty()) {
		selectedGameObject_ = objects_.front(); 
	}

	for (IGameObject* obj : objects_) {
		if (obj == selectedGameObject_) {
			obj->SetEnableOutline(true);
		} else {
			obj->SetEnableOutline(false);
		}
	}

	HandleUndoRedoInput(ctx);

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
	ImGui::BeginChild("ObjectList", ImVec2(0, 400), true);
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

	ImGui::DragFloat3("Direction", &ctx.engineContext->object3dCommon->GetDirectionalLight().direction.x, 0.01f);
	ImGui::ColorEdit4("Color", &ctx.engineContext->object3dCommon->GetDirectionalLight().color.x);
	ImGui::DragFloat("Intensity", &ctx.engineContext->object3dCommon->GetDirectionalLight().intensity, 0.01f);
	ImGui::End();

	// =====================================
	// ギズモ描画レイヤー
	// =====================================
	if (selectedGameObject_ != nullptr) {
		// ウィンドウが折りたたまれている場合は処理をスキップする
		if (!ImGui::Begin("Game")) {
			ImGui::End();
		} else {
			// Gameウィンドウ内の描画領域サイズと位置を取得
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
			ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
			ImVec2 viewPos = ImVec2(windowPos.x + contentMin.x, windowPos.y + contentMin.y);
			ImVec2 viewSize = ImVec2(contentMax.x - contentMin.x, contentMax.y - contentMin.y);

			// 描画領域のサイズが正常な場合のみImGuizmoの描画を行う
			if (viewSize.x > 0.0f && viewSize.y > 0.0f) {
				ImGuizmo::BeginFrame();
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::Enable(true);
				ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

				// ImGuizmoの適用範囲をGameウィンドウのコンテンツエリアに設定
				ImGuizmo::SetRect(viewPos.x, viewPos.y, viewSize.x, viewSize.y);

				Transform& transform = selectedGameObject_->GetTransform();
				float degRot[3] = {
				    transform.rotate.x * 180.0f / std::numbers::pi_v<float>, transform.rotate.y * 180.0f / std::numbers::pi_v<float>, transform.rotate.z * 180.0f / std::numbers::pi_v<float>};

				float objectMatrix[16];
				ImGuizmo::RecomposeMatrixFromComponents(&transform.translate.x, degRot, &transform.scale.x, objectMatrix);
				Matrix4x4 viewMat = ctx.currentCamera->GetViewMatrix();
				Matrix4x4 projMat = ctx.currentCamera->GetProjection();

				// 操作開始の検知
				bool isUsingNow = ImGuizmo::IsUsing();
				if (isUsingNow && !wasGizmoUsing_) {
					gizmoBeginSnapshot_ = CaptureTransform(transform);
				}

				ImGuizmo::Manipulate(&viewMat.m[0][0], &projMat.m[0][0], currentGizmoOperation_, currentGizmoMode_, objectMatrix);

				if (isUsingNow) {
					float newTrans[3], newRot[3], newScale[3];
					ImGuizmo::DecomposeMatrixToComponents(objectMatrix, newTrans, newRot, newScale);

					float oldY = transform.translate.y;
					transform.translate = {newTrans[0], newTrans[1], newTrans[2]};
					transform.translate.y = oldY;
					transform.scale = {newScale[0], newScale[1], newScale[2]};
					transform.rotate = {newRot[0] * std::numbers::pi_v<float> / 180.0f, newRot[1] * std::numbers::pi_v<float> / 180.0f, newRot[2] * std::numbers::pi_v<float> / 180.0f};
				}

				// 操作終了の検知
				if (!isUsingNow && wasGizmoUsing_) {
					TransformSnapshot afterSnap = CaptureTransform(transform);
					PushCommand(std::make_unique<TransformCommand>(selectedGameObject_, gizmoBeginSnapshot_, afterSnap));
				}
				wasGizmoUsing_ = isUsingNow;
			}
			ImGui::End();
		}
	}

	// 選択中オブジェクトのSRT
	ImGui::Begin("Selected GameObject");
	Transform& selTransform = selectedGameObject_->GetTransform();

	if (ImGui::DragFloat3("Scale", &selTransform.scale.x, 0.01f)) {
	}
	if (ImGui::IsItemActivated())
		dragBeginSnapshot_ = CaptureTransform(selTransform);
	if (ImGui::IsItemDeactivatedAfterEdit())
		PushCommand(std::make_unique<TransformCommand>(selectedGameObject_, dragBeginSnapshot_, CaptureTransform(selTransform)));

	if (ImGui::DragFloat3("Rotate", &selTransform.rotate.x, 0.01f)) {
	}
	if (ImGui::IsItemActivated())
		dragBeginSnapshot_ = CaptureTransform(selTransform);
	if (ImGui::IsItemDeactivatedAfterEdit())
		PushCommand(std::make_unique<TransformCommand>(selectedGameObject_, dragBeginSnapshot_, CaptureTransform(selTransform)));

	if (ImGui::DragFloat3("Translate", &selTransform.translate.x, 0.01f)) {
	}
	if (ImGui::IsItemActivated())
		dragBeginSnapshot_ = CaptureTransform(selTransform);
	if (ImGui::IsItemDeactivatedAfterEdit())
		PushCommand(std::make_unique<TransformCommand>(selectedGameObject_, dragBeginSnapshot_, CaptureTransform(selTransform)));

	ImGui::End();
#endif // USE_IMGUI
}

void SceneEditor::UpdatePicking(const SceneContext& ctx) {
#ifdef USE_IMGUI
	if (ImGuizmo::IsOver() || ImGuizmo::IsUsing()) {
		return;
	}

	if (ctx.keyboard->MouseButtonTriggered(0)) {
		// ウィンドウが折りたたまれている場合は取得せず終了
		if (!ImGui::Begin("Game")) {
			ImGui::End();
			return;
		}

		ImVec2 mousePos = ImGui::GetMousePos();
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
		ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
		ImGui::End();

		// Gameウィンドウ内の実際の描画領域の左上座標とサイズ
		ImVec2 viewPos = ImVec2(windowPos.x + contentMin.x, windowPos.y + contentMin.y);
		ImVec2 viewSize = ImVec2(contentMax.x - contentMin.x, contentMax.y - contentMin.y);

		// ウィンドウ内でのローカルマウス座標
		float localMouseX = mousePos.x - viewPos.x;
		float localMouseY = mousePos.y - viewPos.y;

		// マウスカーソルがGameウィンドウの描画領域外にある場合はピッキング処理をしない
		if (localMouseX < 0.0f || localMouseX > viewSize.x || localMouseY < 0.0f || localMouseY > viewSize.y) {
			return;
		}

		// デバイス座標系に正規化
		float nx = (2.0f * localMouseX) / viewSize.x - 1.0f;
		float ny = 1.0f - (2.0f * localMouseY) / viewSize.y;

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
#endif
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

bool SceneEditor::IsObjectAlive(IGameObject* obj) const {
	for (IGameObject* o : objects_) {
		if (o == obj) {
			return true;
		}
	}
	return false;
}

void SceneEditor::PushCommand(std::unique_ptr<IEditorCommand> command) {
	undoStack_.push_back(std::move(command));
	if (undoStack_.size() > kMaxHistorySize) {
		undoStack_.pop_front();
	}
	// 新しい操作が入ったらRedo履歴は破棄
	redoStack_.clear();
}

void SceneEditor::Undo() {
	while (!undoStack_.empty()) {
		auto& command = undoStack_.back();
		if (!IsObjectAlive(command->GetTarget())) {
			// 対象が既に破棄されている場合は無効化して捨てる
			undoStack_.pop_back();
			continue;
		}
		command->Undo();
		redoStack_.push_back(std::move(command));
		undoStack_.pop_back();
		break;
	}
}

void SceneEditor::Redo() {
	while (!redoStack_.empty()) {
		auto& command = redoStack_.back();
		if (!IsObjectAlive(command->GetTarget())) {
			redoStack_.pop_back();
			continue;
		}
		command->Redo();
		undoStack_.push_back(std::move(command));
		redoStack_.pop_back();
		break;
	}
}

void SceneEditor::HandleUndoRedoInput(const SceneContext& ctx) {
#ifdef USE_IMGUI
	// ImGuiがテキスト入力等でキーを消費している場合は無視
	if (ImGui::GetIO().WantTextInput) {
		return;
	}
#endif
	bool ctrlHeld = ctx.keyboard->KeyDown(DIK_LCONTROL) || ctx.keyboard->KeyDown(DIK_RCONTROL);
	if (!ctrlHeld) {
		return;
	}

	if (ctx.keyboard->KeyTriggered(DIK_Z)) {
		Undo();
	}
	if (ctx.keyboard->KeyTriggered(DIK_Y)) {
		Redo();
	}
}