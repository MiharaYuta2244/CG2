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
	Vector3 scale = transform_.scale;
	Vector3 halfScale;
	halfScale.x = scale.x / 2.0f;
	halfScale.y = scale.y / 2.0f;
	halfScale.z = scale.z / 2.0f;
	col_.max = {pos.x + halfScale.x, pos.y + halfScale.y, pos.z + halfScale.z};
	col_.min = {pos.x - halfScale.x, pos.y - halfScale.y, pos.z - halfScale.z};

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
}

void Goal::Draw() {
	// パーティクルの描画
	for (auto& particle : particle_) {
		particle->Draw();
	}
}