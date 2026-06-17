#include "Goal.h"

using namespace TinyEngine;

void Goal::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	transform_.scale = {0.01f, 1.0f, 3.5f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {196.0f, 0.0f, 0.5f};

	// パーティクル生成用のタイマー初期化
	particleGenerateTimer_.Initialize(0.2f);

	// エミッターの初期化
	emitter_.count = 1;
	emitter_.frequency = 2.0f;
	emitter_.frequencyTime = 0.0f;
}

void Goal::Update(float deltaTime) {
	Vector3 pos = transform_.translate;

	// 当たり判定更新
	col_.max = {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f};
	col_.min = {pos.x - 0.5f, pos.y - 0.5f, pos.z - 0.5f};

	// パーティクル生成用のタイマー更新
	particleGenerateTimer_.Update(deltaTime);

	// パーティクルの生成
	if (particleGenerateTimer_.IsEnd()) {
		auto particle = std::make_unique<Particle>();
		particle->Initialize(ctx_, transform_.translate, "Cross.png", std::make_unique<DustModule>(), &emitter_, TinyEngine::ParticleMeshType::Square);
		particle->SetEmitMode(false, 0.1f);
		particle->SetEmitterParam(20, 0.05f);
		particle_.push_back(std::move(particle));

		// タイマーの再設定
		particleGenerateTimer_.Initialize(0.2f);
	}

	// パーティクルの更新
	for (auto& particle : particle_) {
		particle->Update();
	}

	// パーティクル削除処理
	std::erase_if(particle_, [this](const std::unique_ptr<TinyEngine::Particle>& p) { return p->IsFinished(); });

#ifdef USE_IMGUI
	ImGui::Begin("Goal");
	ImGui::DragFloat3("Translate", &transform_.translate.x, 1.0f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::End();

	emitter_.transform = transform_;
#endif // USE_IMGUI
}

void Goal::Draw() {
	// パーティクルの描画
	for (auto& particle : particle_) {
		particle->Draw();
	}
}