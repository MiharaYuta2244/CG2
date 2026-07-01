#include "EnemyBullet.h"
#include "GameObjects/StageObjects/Door/Door.h"
#include "GameObjects/StageObjects/Wall/Wall.h"
#include <cmath>

using namespace TinyEngine;

void EnemyBullet::Initialize(EngineContext* ctx, Vector2 dir, Vector3 pos) {
	// 弾の長さを定義
	float length = 15.0f;
	float width = 0.5f;

	// 移動方向を受け取る
	direction_ = dir;

	// プレイヤーの方向に向けて回転させる
	transform_.rotate.y = std::atan2(dir.x, dir.y);

	// 動かない弾として、敵の位置からプレイヤー方向へ伸ばす
	Vector3 centerOffset = {dir.x * margin_, 0.0f, dir.y * margin_};
	transform_.translate = {pos.x + centerOffset.x, pos.y, pos.z + centerOffset.z};

	// スケールを設定
	transform_.scale = {width, 1.0f, length};

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<VisionCone>();
	render_->Initialize(ctx, length, 20.0f);
	render_->SetColor({1, 1, 1, 1});
	Transform transform = transform_;
	transform.translate =  pos - centerOffset;
	render_->SetTransform(transform);

	// タイマーのリセット
	deathTimer_ = 0.0f;

	// OBBの当たり判定
	col_.center = transform_.translate;
	col_.size = transform_.scale;

	// Y軸回転のみを考慮したローカル軸の計算
	float cosY = std::cos(transform_.rotate.y);
	float sinY = std::sin(transform_.rotate.y);

	// X軸
	col_.orientations[0] = {cosY, 0.0f, -sinY};
	// Y軸
	col_.orientations[1] = {0.0f, 1.0f, 0.0f};
	// Z軸
	col_.orientations[2] = {sinY, 0.0f, cosY};
}

void EnemyBullet::Update(float deltaTime, float bulletSpeed) {
	deathTimer_ += deltaTime;

	// 描画用インスタンス更新
	std::list<std::unique_ptr<Wall>> emptyWalls;
	std::list<std::unique_ptr<Door>> emptyDoors;
	render_->Update(emptyWalls, emptyDoors);
}

void EnemyBullet::Draw() {
	// 描画
	render_->Draw();
}

bool EnemyBullet::IsDead(const Rect<float>& rect) const {
	// 一定時間経過していたら削除フラグを立てる
	return deathTimer_ >= kLifeTime_;
}