#include "Player.h"
#include "ColorPalette.h"
#include "GameObjects/Effect/EffectGenerator.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Enemy/EnemyManager.h"

using namespace TinyEngine;

void Player::Initialize(EngineContext* ctx, TinyEngine::DecalManager* bloodDecalManager) {
	ctx_ = ctx;

	transform_.scale = {3.0f, 3.0f, 3.0f};
	transform_.rotate = {0.0f, std::numbers::pi_v<float> / 2.0f, 0.0f};
	transform_.translate = {0.0f, 0.0f, 0.0f};

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "GorillaIdle.gltf");
	KeyframeAnimation keyframeAnimation;
	Animation sneakWalkAnimation = keyframeAnimation.LoadAnimationFile("GorillaIdle.gltf");
	render_->GetObject3d()->PlayAnimation(sneakWalkAnimation);
	render_->SetColor(color_);
	render_->SetTransform(transform_);
	render_->SetEnvScale(envScale_);
	render_->SetIsSkinning(true);

	// 移動用インスタンス生成
	move_ = std::make_unique<PlayerMove>();

	// HP管理用インスタンス生成&初期化
	hp_ = std::make_unique<PlayerHealth>();
	hp_->Initialize(maxHP_);

	// パーティクル生成タイマー初期化
	particleGenerateTimer_.Initialize(0.2f);

	// プレイヤーのHPゲージ生成&初期化
	hpIcon_ = std::make_unique<PlayerHPIcon>();
	hpIcon_->Initialize(ctx);

	// 血痕の管理インスタンスポインタ
	bloodDecalManager_ = bloodDecalManager;
}

void Player::Update(float deltaTime, DirectInput* input, GamePad* gamePad, EnemyManager* enemyManager) {
	// アクションアニメーションのタイマー更新
	if (isActionAnimating_) {
		actionAnimTimer_ -= deltaTime;
		if (actionAnimTimer_ <= 0.0f) {
			isActionAnimating_ = false; // 指定時間経過でアクション終了
		}
	}

	// アナログ入力ベクトル
	Vector2 inputDir = {0.0f, 0.0f};
	Vector2 aimDir = {0.0f, 0.0f};

	if (input->KeyDown(DIK_D))
		inputDir.x += 1.0f;
	if (input->KeyDown(DIK_A))
		inputDir.x -= 1.0f;
	if (input->KeyDown(DIK_W))
		inputDir.y += 1.0f;
	if (input->KeyDown(DIK_S))
		inputDir.y -= 1.0f;

	// ゲームパッド入力
	if (gamePad && gamePad->GetState().connected) {
		const auto& padState = gamePad->GetState();

		// 十字キー
		if (padState.buttons.dpadRight)
			inputDir.x += 1.0f;
		if (padState.buttons.dpadLeft)
			inputDir.x -= 1.0f;
		if (padState.buttons.dpadUp)
			inputDir.y += 1.0f;
		if (padState.buttons.dpadDown)
			inputDir.y -= 1.0f;

		// 左スティック入力（デッドゾーンを超えていれば上書き）
		if (std::abs(padState.axes.lx) > 0.1f || std::abs(padState.axes.ly) > 0.1f) {
			inputDir.x = padState.axes.lx;
			inputDir.y = -padState.axes.ly;
		}

		// 右スティック入力
		if (std::abs(padState.axes.rx) > 0.1f || std::abs(padState.axes.ry) > 0.1f) {
			aimDir.x = padState.axes.rx;
			aimDir.y = -padState.axes.ry;
		}
	}

	// ベクトルの長さを計算し、斜め移動時などに長さが1.0を超えないようにクランプ
	float length = std::sqrtf(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
	if (length > 1.0f) {
		inputDir.x /= length;
		inputDir.y /= length;
	}

	// 移動状態の更新と向きの記録
	if (length > 0.05f) {
		// 正規化して記録（攻撃・投げの方向用）
		lastMoveDirection_ = {inputDir.x / length, inputDir.y / length};
		isMoving_ = true;

		// アクションアニメーション中でない場合のみ、歩きモデルに切り替え
		if (!isActionAnimating_) {
			std::string targetModel = isHold_ ? "GorillaHoldWalking.gltf" : "Gorilla.gltf";
			if (render_->GetFilepath() != targetModel) {
				render_->SetModel(targetModel);
				KeyframeAnimation keyframeAnimation;
				Animation walkAnimation = keyframeAnimation.LoadAnimationFile(targetModel);
				render_->GetObject3d()->PlayAnimation(walkAnimation);
			}
		}
	} else {
		isMoving_ = false;

		// アクションアニメーション中でない場合のみ、待機モデルに切り替え
		if (!isActionAnimating_) {
			std::string targetModel = isHold_ ? "GorillaHolding.gltf" : "GorillaIdle.gltf";
			if (render_->GetFilepath() != targetModel) {
				render_->SetModel(targetModel);
				KeyframeAnimation keyframeAnimation;
				Animation idleAnimation = keyframeAnimation.LoadAnimationFile(targetModel);
				render_->GetObject3d()->PlayAnimation(idleAnimation);
			}
		}
	}

	// 敵を掴んでいる場合は速度を半減させる
	speedMultiplier_ = isHold_ ? 0.3f : 1.0f;

	if (!hp_->IsDead()) {
		// 移動更新
		move_->Update(&transform_, inputDir, aimDir, deltaTime, speedMultiplier_, isHold_);

		// 攻撃等に使うベクトルの更新
		lastMoveDirection_.x = std::sin(transform_.rotate.y);
		lastMoveDirection_.y = std::cos(transform_.rotate.y);
	}

	// HP管理インスタンス更新
	hp_->Update(deltaTime);

	if (ctx_ && ctx_->object3dCommon) {
		PointLight pointLight = ctx_->object3dCommon->GetPointLight();

		// パラメータ
		pointLight.position = transform_.translate;
		pointLight.position.y += pointLightHeightOffset_;
		pointLight.color = pointLightColor_;
		pointLight.intensity = pointLightIntensity_;
		pointLight.radius = pointLightRadius_;
		pointLight.decay = pointLightDecay_;

		ctx_->object3dCommon->SetPointLightParam(pointLight);
	}

	// 衝突判定のために、移動後の座標で仮のAABBの更新
	UpdateCollision();

	// 掴み判定用：一番近い敵を探す
	Enemy* targetEnemy = nullptr;

	// 敵を掴んでいない時のみ新しいターゲットを探す
	if (!isHold_) {
		float minDist = FLT_MAX;
		for (auto& enemy : enemyManager->GetEnemies()) {
			if (enemy->IsDead())
				continue;

			// プレイヤーと敵の距離を計算
			Vector3 ePos = enemy->GetPos();
			Vector3 diff = {ePos.x - transform_.translate.x, ePos.y - transform_.translate.y, ePos.z - transform_.translate.z};
			float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

			if (dist < minDist && dist < grabRange_) {
				minDist = dist;
				targetEnemy = enemy.get();
			}
		}
	}

	isGrab_ = input->KeyDown(DIK_J);
	isGrabReleased_ = input->KeyReleased(DIK_J);
	isAttackTriggered_ = input->KeyTriggered(DIK_K);

	if (gamePad && gamePad->GetState().connected) {
		const auto& padState = gamePad->GetState();

		// Lトリガーで掴み
		if (padState.axes.lt > 0.3f) {
			isGrab_ = true;
		}
		// 放す
		if (isHold_ && !isGrab_) {
			isGrabReleased_ = true;
		}
		// Rトリガーで突き飛ばし
		if (padState.axes.rt > 0.3f) {
			isAttackTriggered_ = true;
		}
	}

	// 掴み・投げ処理の更新
	if (enableAttack_ && isGrab_) {
		if (!isHold_ && targetEnemy != nullptr) {
			isHold_ = true;

			heldEnemy_ = targetEnemy;
			heldEnemy_->SetEnableAI(false);
			heldEnemy_->SetShotHoldState(true);
			heldEnemy_->SetAIState(EnemyAI::State::Hold);
			heldEnemy_->ResetShotTimer();
			isGrabTriggered_ = true;

			if (render_->GetFilepath() != "GorillaHold.gltf") {
				render_->SetModel("GorillaHold.gltf");
				KeyframeAnimation keyframeAnimation;
				Animation holdAnimation = keyframeAnimation.LoadAnimationFile("GorillaHold.gltf");
				render_->GetObject3d()->PlayAnimation(holdAnimation);
				isActionAnimating_ = true;
				actionAnimTimer_ = 0.2f;
			}
		}

		// 掴んでいる間はプレイヤーの位置に敵を固定
		if (isHold_ && heldEnemy_ != nullptr) {
			Vector3 forward = {lastMoveDirection_.x, 0.0f, lastMoveDirection_.y};

			// 前方オフセット
			Vector3 holdPos = {transform_.translate.x + forward.x * attackOffset_, transform_.translate.y, transform_.translate.z + forward.z * attackOffset_};

			heldEnemy_->SetPos(holdPos);
			heldEnemy_->SetRotate(transform_.rotate);
		}
	} else if (isHold_ && isGrabReleased_) { // 手放す処理
		isHold_ = false;
		if (heldEnemy_) {
			heldEnemy_->SetEnableAI(true);
			heldEnemy_->SetAIState(EnemyAI::State::Normal);
			heldEnemy_ = nullptr; // 手放す
		}
	}

	// 投げる・攻撃処理
	if (enableAttack_ && isAttackTriggered_) {
		// 投げる対象を決定する（掴んでいる敵がいればそれ、いなければ近くの敵）
		Enemy* throwTarget = nullptr;

		if (isHold_ && heldEnemy_ != nullptr) {
			// 掴んでいる敵を投げる場合
			throwTarget = heldEnemy_;
			isHold_ = false;
			heldEnemy_ = nullptr; // 投げたのでポインタをクリア
		} else if (!isHold_ && targetEnemy != nullptr) {
			// 掴んでいないが、近くにいる敵を直接投げる場合
			throwTarget = targetEnemy;
		}

		// 投げる対象がいればノックバック処理を実行
		if (throwTarget != nullptr) {
			// enableAttack_ = false;
			throwTarget->SetEnableAI(true);
			throwTarget->StartKnockBack({lastMoveDirection_.x, 0.0f, lastMoveDirection_.y});
			throwTarget->SetAIState(EnemyAI::State::Normal);
		}

		// アニメーション切り替え
		if (render_->GetFilepath() != "GorillaPush.gltf") {
			render_->SetModel("GorillaPush.gltf");
			KeyframeAnimation keyframeAnimation;
			Animation pushAnimation = keyframeAnimation.LoadAnimationFile("GorillaPush.gltf");
			render_->GetObject3d()->PlayAnimation(pushAnimation);

			// アクションアニメーションの再生開始を記録し、タイマーを設定
			isActionAnimating_ = true;
			actionAnimTimer_ = 0.2f;
		}
	}

	// パーティクルの生成タイマー更新
	if (isMoving_) {
		particleGenerateTimer_.Update(deltaTime);
	}

	// パーティクルの生成
	if (particleGenerateTimer_.IsEnd() && !hp_->IsDead()) {
		auto particle = std::make_unique<Particle>();
		particle->Initialize(ctx_, render_->GetBonePos(L"ボーン.022"), "Dust.png", std::make_unique<DustStepModule>(), nullptr, TinyEngine::ParticleMeshType::Square);
		particle->SetEmitMode(false, 0.1f);
		particle->SetEmitterParam(1, 0.3f);
		dustParticle_.push_back(std::move(particle));

		// タイマーの再設定
		particleGenerateTimer_.Initialize(0.2f);
	}

	// パーティクルの更新
	for (auto& particle : dustParticle_) {
		particle->Update();
	}

	// パーティクル削除処理
	std::erase_if(dustParticle_, [this](const std::unique_ptr<TinyEngine::Particle>& p) { return p->IsFinished(); });

	// ヒットエフェクトの更新
	for (auto& particle : hitEffects_) {
		particle->Update();
	}

	// ヒットエフェクト削除
	std::erase_if(hitEffects_, [](const std::unique_ptr<TinyEngine::Particle>& p) { return p->IsFinished(); });

	// 回復エフェクトの更新
	for (auto& particle : healEffects_) {
		particle->UpdateTranslate(transform_.translate);
		particle->Update();
	}

	// 回復エフェクト削除
	std::erase_if(healEffects_, [](const std::unique_ptr<TinyEngine::Particle>& p) { return p->IsFinished(); });

	// プレイヤーのHPゲージ更新
	hpIcon_->Update(deltaTime);

	// 出血処理
	Bleeding(deltaTime);

	// ギズモ用当たり判定更新
	UpdateAABBForGizmo();
}

void Player::PostUpdate() {
	// 押し戻しによってtransform_が変更された場合、それに合わせてAABBも最終的な正しい位置に合わせる
	UpdateCollision();

	// 最後に描画用の更新を行う
	render_->Update(transform_);
}

void Player::Draw() {
	// パーティクルの描画
	for (auto& particle : dustParticle_) {
		particle->Draw();
	}

	// ヒットエフェクトパーティクルの描画
	for (auto& particle : hitEffects_) {
		particle->Draw();
	}

	// 回復エフェクトの描画
	for (auto& particle : healEffects_) {
		particle->Draw();
	}

	// 描画
	render_->Draw();

	// プレイヤーHPのUI描画
	hpIcon_->Draw();
}

bool Player::IsDead() const {
	// ゲーム開始から死亡までの時間をログに出力
	Logger::Log("", LogLevel::Info);

	return hp_->IsDead();
}

void Player::Damage(float value) {
	if (hp_->GetIsInvincible())
		return;

	// ダメージ前のHPを保存
	float beforeHP = hp_->GetCurrentHP();

	// ダメージ処理
	hp_->Damage(value);

	// ダメージ後のHP
	float afterHP = hp_->GetCurrentHP();

	// ヒットエフェクト生成
	GenerateHitEffect();

	// HPIconのアニメーション開始処理
	int startIdx = static_cast<int>(afterHP);
	int endIdx = static_cast<int>(beforeHP);

	for (int i = startIdx; i < endIdx; ++i) {
		hpIcon_->DmageAnimStart(i);
	}

	// 血痕の生成
	AddBloodDecal({4, 4, 1});

	// 出血用のタイマー初期化
	bleedingTimer_.Initialize(0.5f);
}

void Player::Heal(float value) {
	// 回復前のHPを保存
	float beforeHP = hp_->GetCurrentHP();

	// 回復処理
	hp_->Heal(value);

	// 回復後のHP
	float afterHP = hp_->GetCurrentHP();

	if (beforeHP != afterHP) {
		// エフェクト生成
		GenerateHealEffect();
	}

	// HPIconのアニメーション開始処理
	int startIdx = static_cast<int>(beforeHP);
	int endIdx = static_cast<int>(afterHP);

	for (int i = startIdx; i < endIdx; ++i) {
		hpIcon_->HealAnimStart(i);
	}
}

void Player::AllHeal() {
	// 回復前のHPを保存
	float beforeHP = hp_->GetCurrentHP();

	// 全回復処理
	hp_->AllHeal();

	// 回復後のHP
	float afterHP = hp_->GetCurrentHP();

	if (beforeHP != afterHP) {
		// エフェクト生成
		GenerateHealEffect();
	}

	// HPIconのアニメーション開始処理
	int startIdx = static_cast<int>(beforeHP);
	int endIdx = static_cast<int>(afterHP);

	for (int i = startIdx; i < endIdx; ++i) {
		hpIcon_->HealAnimStart(i);
	}
}

void Player::UpdateCollision() {
	// 攻撃用の当たり判定更新
	Vector3 pos = transform_.translate;
	Vector3 forward = {std::sin(transform_.rotate.y), 0.0f, std::cos(transform_.rotate.y)};
	Vector3 right = {forward.z, 0.0f, -forward.x};
	Vector3 up = {0.0f, 1.0f, 0.0f};

	float attackLength = 1.5f; // 前方に伸びる距離
	float attackWidth = 0.6f;  // 横幅
	float attackHeight = 1.0f; // 高さ

	Vector3 center = {pos.x + forward.x * attackLength * 0.5f, pos.y, pos.z + forward.z * attackLength * 0.5f};
	attackCol_.center = center;
	attackCol_.orientations[0] = right;
	attackCol_.orientations[1] = up;
	attackCol_.orientations[2] = forward;
	attackCol_.size = {attackWidth, attackHeight, attackLength};

	// 本体の当たり判定更新
	bodyCol_.max = {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f};
	bodyCol_.min = {pos.x - 0.5f, pos.y - 0.5f, pos.z - 0.5f};
}

void Player::GenerateHitEffect() {
	if (preHP_ == hp_->GetCurrentHP())
		return;

	// エフェクトの生成
	EffectGenerator::CreateHitEffect(ctx_, transform_.translate, hitEffects_);

	preHP_ = hp_->GetCurrentHP();
}

void Player::AddBloodDecal(Vector3 scale) {
	Vector3 basePos = transform_.translate;
	Vector3 finalPos;
	finalPos.x = basePos.x;
	finalPos.y = 0.1f;
	finalPos.z = basePos.z;
	bloodDecalManager_->AddDecal("Bleeding.png", finalPos, {std::numbers::pi_v<float> / 2.0f, 0, 0}, scale, ColorPalette::NeonRed());
}

void Player::Bleeding(float deltaTime) {
	if (hp_->GetCurrentHP() <= 2 && isMoving_) {
		if (bleedingTimer_.IsEnd()) {
			// スケールの最大値と最小値
			Vector3 bloodScaleMin = {0.5f, 0.5f, 0.5f};
			Vector3 bloodScaleMax = {1.0f, 1.0f, 1.0f};

			// 生成間隔の最大値と最小値
			float intervalMin = 0.2f;
			float intervalmax = 0.5f;

			float hp = hp_->GetCurrentHP();
			float t = hp / 2.0f;

			// スケールの確定
			Vector3 scale = bloodScaleMax - (bloodScaleMax - bloodScaleMin) * t;

			// 生成間隔の確定
			float interval = intervalMin + (intervalmax - intervalMin) * t;

			// 血痕の生成
			AddBloodDecal(scale);

			// 出血用のタイマー初期化
			bleedingTimer_.Initialize(interval);
		}

		// 出血用タイマー更新
		bleedingTimer_.Update(deltaTime);
	}
}

void Player::GenerateHealEffect() {
	// エフェクトの生成
	EffectGenerator::CreateHealEffect(ctx_, transform_.translate, healEffects_);

	// 追従フラグを立てる
	for (auto& effect : healEffects_) {
		effect->SetFollowTarget(true);
	}
}

void Player::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Player PointLight");

	ImGui::DragFloat("Height Offset", &pointLightHeightOffset_, 0.1f, -5.0f, 10.0f);
	ImGui::ColorEdit4("Color", &pointLightColor_.x);
	ImGui::DragFloat("Intensity", &pointLightIntensity_, 0.05f, 0.0f, 20.0f);
	ImGui::DragFloat("Radius", &pointLightRadius_, 0.5f, 0.0f, 100.0f);
	ImGui::DragFloat("Decay", &pointLightDecay_, 0.05f, 0.0f, 10.0f);

	ImGui::End();
#endif
}