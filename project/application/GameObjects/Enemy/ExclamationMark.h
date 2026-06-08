#pragma once
#include "AnimationBundle.h"
#include "GameObjects/ObjectRender/ObjectRender.h"

/// <summary>
/// プレイヤー発見時に頭上に出す感嘆符 「!」マーク
/// </summary>
class ExclamationMark {
public:
	// 初期化
	void Initialize(EngineContext* ctx, Vector3 translate);

	// 更新
	void Update(float deltaTime, Vector3 enemyPos);

	// 描画
	void Draw();

	// アニメーション終了フラグ
	bool IsFinishedAnimation() const { return scaleAnim_.anim.GetIsActive(); }

private:
	// 描画用インスタンス
	std::unique_ptr<ObjectRender> render_;

	// Transform
	Transform transform_;

	// スケールイージングアニメーション
	AnimationBundle<float> scaleAnim_;

	// 座標
	Vector3 pos_;

	// 余白
	Vector3 margin_;
};
