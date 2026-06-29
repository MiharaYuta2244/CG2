#include "Door.h"
#include "MathOperator.h"

void Door::Initialize(EngineContext* ctx, DoorStatus doorStatus) {
	transform_.scale = {doorStatus.width, 100.0f, doorStatus.depth};
	transform_.rotate = {0.0f, doorStatus.rotateY, 0.0f};
	transform_.translate = {doorStatus.centerX, 0.0f, doorStatus.centerZ};
	doorStatus_ = doorStatus;

	// 描画用インスタンス生成&初期化
	for (int i = 0; i < renders_.size(); ++i) {
		renders_[i] = std::make_unique<ObjectRender>();
		renders_[i]->Initialize(ctx, "Cube.obj");
		renders_[i]->SetColor(color_);
		renders_[i]->SetEnableLighting(false);
	}

	CulcTransform();
}

void Door::Update(float deltaTime, Vector3 playerPos) {
	// 中心点の計算
	Vector3 centerPos = {
	    (transform_.translate.x + transformSecond_.translate.x) * 0.5f, (transform_.translate.y + transformSecond_.translate.y) * 0.5f, (transform_.translate.z + transformSecond_.translate.z) * 0.5f};

	// 中心点とplayerPosとの距離を計算
	Vector3 diff = MathUtility::Subtract(centerPos, playerPos);
	float distance = MathUtility::Length(diff);

	// 一定の距離に近づいたらisOpen_フラグを立てる
	if (distance <= targetDistance_) {
		isOpen_ = true;
	} else {
		isOpen_ = false;
	}

	float sinY = std::sin(transform_.rotate.y);
	float cosY = std::cos(transform_.rotate.y);
	Vector3 moveDir = {sinY, 0.0f, cosY};

	// 前回の移動分を引く
	transform_.translate.x -= moveDir.x * previousOpenOffset_;
	transform_.translate.z -= moveDir.z * previousOpenOffset_;
	transformSecond_.translate.x += moveDir.x * previousOpenOffset_;
	transformSecond_.translate.z += moveDir.z * previousOpenOffset_;

	// 基準となる初期位置を計算
	CulcTransform();

	// 開閉状態が切り替わった瞬間を検知して、アニメーションをセットする
	if (isOpen_ != wasOpen_) {
		wasOpen_ = isOpen_; // 状態を更新

		float startValue = openOffset_;                   // 現在の開き具合をスタート地点にする（途中で切り替わった場合の対策）
		float endValue = isOpen_ ? maxOpenOffset_ : 0.0f; // 開くなら最大値、閉まるなら0

		// イージングアニメーションの初期化
		openAnimBundle_.anim = EasingAnimation<float>(startValue, endValue, animDuration_, EaseType::EASEOUTEXPO);
		openAnimBundle_.start = startValue;
		openAnimBundle_.end = endValue;
	}

	// アニメーション更新
	openAnimBundle_.anim.Update(deltaTime, openOffset_);

	// 計算した開き具合をそれぞれ適用する
	transform_.translate.x += moveDir.x * openOffset_;
	transform_.translate.z += moveDir.z * openOffset_;
	transformSecond_.translate.x -= moveDir.x * openOffset_;
	transformSecond_.translate.z -= moveDir.z * openOffset_;

	// 現在の状態を記録
	previousOpenOffset_ = openOffset_;

	// 当たり判定の更新
	UpdateCollision();

	// 描画用インスタンス更新
	renders_[0]->Update(transform_);
	renders_[1]->Update(transformSecond_);
}

void Door::Draw() {
	for (const auto& render : renders_) {
		render->Draw();
	}
}

void Door::SetDoorStatus(DoorStatus doorStatus) {
	transform_.scale.x = doorStatus.width;
	transform_.scale.z = doorStatus.depth;
	transform_.translate.x = doorStatus.centerX;
	transform_.translate.z = doorStatus.centerZ;
	transform_.rotate.y = doorStatus.rotateY;
	doorStatus_ = doorStatus;
}

void Door::CulcTransform() {
	transformSecond_ = transform_;

	// 2枚目のドアの配置オフセットをY軸で回転させる
	float sinY = std::sin(transform_.rotate.y);
	float cosY = std::cos(transform_.rotate.y);

	// marginPos_のベクトルをY軸回転させたもの
	Vector3 rotatedMargin = {marginPos_.z * sinY, marginPos_.y, marginPos_.z * cosY};

	// 修正されたマージンを使って位置を計算
	transformSecond_.translate = transform_.translate - rotatedMargin;

	renders_[0]->SetTransform(transform_);
	renders_[1]->SetTransform(transformSecond_);
}

void Door::UpdateCollision() {
	// 1枚目のドアのAABB
	Vector3 halfSize1 = {
	    transform_.scale.x * 0.5f,
	    transform_.scale.y * 0.5f,
	    transform_.scale.z * 0.5f,
	};

	AABB door1;
	door1.min = {
	    transform_.translate.x - halfSize1.x,
	    transform_.translate.y - halfSize1.y,
	    transform_.translate.z - halfSize1.z,
	};
	door1.max = {
	    transform_.translate.x + halfSize1.x,
	    transform_.translate.y + halfSize1.y,
	    transform_.translate.z + halfSize1.z,
	};

	// 2枚目のドアのAABB
	Vector3 halfSize2 = {
	    transformSecond_.scale.x * 0.5f,
	    transformSecond_.scale.y * 0.5f,
	    transformSecond_.scale.z * 0.5f,
	};

	AABB door2;
	door2.min = {
	    transformSecond_.translate.x - halfSize2.x,
	    transformSecond_.translate.y - halfSize2.y,
	    transformSecond_.translate.z - halfSize2.z,
	};
	door2.max = {
	    transformSecond_.translate.x + halfSize2.x,
	    transformSecond_.translate.y + halfSize2.y,
	    transformSecond_.translate.z + halfSize2.z,
	};

	// 2枚のドアをまとめたAABB
	collision_.min.x = std::min(door1.min.x, door2.min.x);
	collision_.min.y = std::min(door1.min.y, door2.min.y);
	collision_.min.z = std::min(door1.min.z, door2.min.z);

	collision_.max.x = std::max(door1.max.x, door2.max.x);
	collision_.max.y = std::max(door1.max.y, door2.max.y);
	collision_.max.z = std::max(door1.max.z, door2.max.z);
}