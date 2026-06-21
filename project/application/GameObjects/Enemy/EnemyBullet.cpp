#include "EnemyBullet.h"
#include <algorithm>
#include <cmath>

void EnemyBullet::Initialize(EngineContext* ctx, Vector2 dir, Vector3 pos) {
	// 弾の長さを定義
	float length = 15.0f;
	float width = 0.1f;

	// 移動方向を受け取る
	direction_ = dir;

	// プレイヤーの方向に向けて回転させる
	transform_.rotate.y = std::atan2(dir.x, dir.y);

	// 動かない弾として、敵の位置からプレイヤー方向へ伸ばす
	Vector3 centerOffset = {dir.x * (length / 2.0f), 0.0f, dir.y * (length / 2.0f)};
	transform_.translate = {pos.x + centerOffset.x, pos.y, pos.z + centerOffset.z};

	// スケールを設定
	transform_.scale = {width, 1.0f, length};

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");

	// タイマーのリセット
	deathTimer_ = 0.0f;

	// 始点と終点
	Vector3 startPos = pos;
	Vector3 endPos = {pos.x + dir.x * length, pos.y, pos.z + dir.y * length};

	// 始点と終点を内包し、幅を持たせた最大の矩形をAABBとする
	col_.min.x = std::min(startPos.x, endPos.x) - width / 2.0f;
	col_.min.y = pos.y - 0.5f;
	col_.min.z = std::min(startPos.z, endPos.z) - width / 2.0f;

	col_.max.x = std::max(startPos.x, endPos.x) + width / 2.0f;
	col_.max.y = pos.y + 0.5f;
	col_.max.z = std::max(startPos.z, endPos.z) + width / 2.0f;
}

void EnemyBullet::Update(float deltaTime, float bulletSpeed) {
	deathTimer_ += deltaTime;

	// 描画用インスタンス更新
	render_->Update(transform_);
}

void EnemyBullet::Draw() {
	// 描画
	render_->Draw();
}

bool EnemyBullet::IsDead(const Rect<float>& rect) const {
	// 一定時間経過していたら削除フラグを立てる
	return deathTimer_ >= kLifeTime_;
}