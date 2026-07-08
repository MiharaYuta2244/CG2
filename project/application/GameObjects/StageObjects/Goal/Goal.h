#pragma once
#include "AABB.h"
#include "GameObjects/IGameObject.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "GameTimer.h"
#include "Particle.h"

/// <summary>
/// ゴール判定用クラス
/// </summary>
class Goal : public IGameObject {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// 当たり判定Getter
	AABB GetCol() const { return col_; }

	// ゴールフラグSetter
	void SetGoal(bool isGoal) { isGoal_ = isGoal; }

	// ゴールフラグGetter
	bool GetGoal() const { return isGoal_; }

	std::string GetName() const { return "Goal"; }

private:
	EngineContext* ctx_ = nullptr;
	AABB col_;
	bool isGoal_ = false; // ゴールフラグ

	std::vector<std::unique_ptr<TinyEngine::Particle>> particle_; // パーティクル
	GameTimer particleGenerateTimer_;                             // パーティクル生成用のタイマー
	TinyEngine::Particle::Emitter emitter_;                       // エミッター
};
