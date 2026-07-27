#pragma once
#include "GameObjects/PlaceableObject.h"
#include "JsonManager.h"
#include "Transform.h"
#include <list>
#include <memory>
#include <string>
#include <type_traits>

template<typename T, typename... ExtraArgs> class GameObjectManager {
	static_assert(std::is_base_of<PlaceableObject, T>::value, "T must derive from PlaceableObject");

public:
	// 初期化処理
	void Initialize(EngineContext* ctx, std::string windowName, std::string jsonFileName, Transform defaultTransform = {}, ExtraArgs... extraArgs) {
		ctx_ = ctx;
		windowName_ = std::move(windowName);
		jsonPath_ = std::move(jsonFileName);
		defaultTransform_ = defaultTransform;
		extraArgs_ = std::make_tuple(extraArgs...);
		LoadFromJson(jsonPath_);
	}

	// 更新処理
	template<typename... UpdateArgs> void Update(UpdateArgs&&... args) {
		for (auto& obj : objects_) {
			obj->Update(std::forward<UpdateArgs>(args)...);
		}
	}

	// 描画処理
	void Draw() {
		for (auto& obj : objects_) {
			obj->Draw();
		}
	}

	// ImGui描画
	void DrawImGui() {
#ifdef USE_IMGUI
		ImGui::Begin(windowName_.c_str());
		ImGui::Text("Count: %d", (int)objects_.size());
		ImGui::Separator();

		int index = 0;
		for (auto it = objects_.begin(); it != objects_.end();) {
			auto& obj = *it;
			Transform& t = obj->GetTransform();

			ImGui::PushID(index);
			std::string header = "Object " + std::to_string(index);
			if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::DragFloat3("Translate", &t.translate.x, 0.1f);
				ImGui::DragFloat3("Rotate", &t.rotate.x, 0.01f);
				ImGui::DragFloat3("Scale", &t.scale.x, 0.1f);

				if (ImGui::Button("Delete")) {
					it = objects_.erase(it);
					ImGui::PopID();
					continue;
				}
			}
			ImGui::PopID();
			++it;
			++index;
		}

		ImGui::Separator();
		if (ImGui::Button("Add")) {
			objects_.push_back(CreateObject(defaultTransform_));
		}
		if (ImGui::Button("Save JSON")) {
			SaveToJson(jsonPath_);
		}
		ImGui::End();
#endif
	}

	// オブジェクト削除
	void RemoveObject(T* target) {
		for (auto it = objects_.begin(); it != objects_.end(); ++it) {
			if (it->get() == target) {
				objects_.erase(it);
				break;
			}
		}
	}

	// オブジェクトのリストGetter
	const std::list<std::unique_ptr<T>>& GetObjects() const { return objects_; }

private:
	std::unique_ptr<T> CreateObject(const Transform& t) {
		auto obj = std::make_unique<T>();
		std::apply([&](auto&&... args) { obj->Initialize(ctx_, t, args...); }, extraArgs_);
		return obj;
	}

	// 読み込み処理
	void LoadFromJson(const std::string& filepath) {
		objects_.clear();
		std::vector<Transform> transformList;
		if (!JsonManager::Load(filepath, transformList)) {
			return;
		}
		for (const auto& t : transformList) {
			objects_.push_back(CreateObject(t));
		}
	}

	// 保存処理
	void SaveToJson(const std::string& filepath) {
		std::vector<Transform> transformList;
		for (auto& obj : objects_) {
			transformList.push_back(obj->GetTransform());
		}
		JsonManager::Save(filepath, transformList);
	}

private:
	EngineContext* ctx_ = nullptr;
	std::string windowName_;
	std::string jsonPath_;
	Transform defaultTransform_{};
	std::tuple<ExtraArgs...> extraArgs_;
	std::list<std::unique_ptr<T>> objects_;
};