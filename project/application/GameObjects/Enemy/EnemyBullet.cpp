#include "EnemyBullet.h"
#include <cmath>

void EnemyBullet::Initialize(EngineContext* ctx, Vector2 dir, Vector3 pos) {
	// 移動方向を受け取る
	direction_ = dir;

	// プレイヤーの方向に向けて回転させる
	transform_.rotate.y = std::atan2(dir.x, dir.y);

	// 動かない弾として、敵の位置からプレイヤー方向へ伸ばす
	Vector3 centerOffset = {dir.x * margin_, 0.0f, dir.y * margin_};
	transform_.translate = {pos.x + centerOffset.x, pos.y, pos.z + centerOffset.z};

	// スケールを設定
	transform_.scale = {0.5f, 0.5f, 15.0f};

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cylinder.obj");
	render_->SetEnableLaser(true);
	render_->SetColor({0.0f, 0.0f, 1.0f, 1.0f});

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

	// 描画用のサイズを定義
	float width = 0.2f;
	float height = 0.2f;

	// アニメーション開始処理
	scaleXYAnim_.anim.Start({width, height}, {0.0f, 0.0f}, kLifeTime_, EaseType::EASEINCUBIC);

	// フレーム数のリセット
	aliveFrameCount_ = 0;
}

void EnemyBullet::Update(float deltaTime) {
	deathTimer_ += deltaTime;
	laserTimer_ += deltaTime;

	// フレーム数の加算
	aliveFrameCount_++;

	bool isPlaying = scaleXYAnim_.anim.Update(deltaTime, scaleXYAnim_.temp);
	if (isPlaying) {
		Vector3 scale = {scaleXYAnim_.temp.x, scaleXYAnim_.temp.y, transform_.scale.z};
		transform_.scale = scale;
	}

	// 描画用インスタンス更新
	render_->SetTime(laserTimer_);
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