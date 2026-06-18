#include "particle.h"
#include "Collision.h"
#include "DirectXUtils.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "Model.h"
#include "ParticleModule.h"
#include "Random.h"
#include "TextureManager.h"
#include "particleCommon.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace TinyEngine;

Particle::~Particle(){
	if (ctx_ && ctx_->srvManager) {
		ctx_->srvManager->Free(srvIndex_);
	}
}

void Particle::Initialize(
    EngineContext* ctx, Vector3 emitterPos, const std::string& texturePath, std::unique_ptr<ParticleModule> module, const Emitter* customEmitter,
    ParticleMeshType meshType) {
	ctx_ = ctx;

	// モジュールのセット
	module_ = std::move(module);

	// インスタンシングデータ作成
	CreateInstancingResource();

	// SRVの作成（インスタンシング用）
	srvIndex_ = ctx_->srvManager->Allocate();
	CreateInstancingSRV(srvIndex_);

	// 指定されたメッシュタイプに応じてモデルデータを作成
	switch (meshType) {
	case TinyEngine::ParticleMeshType::Square:
		modelData_ = CreatePrimitive(texturePath);
		break;
	case TinyEngine::ParticleMeshType::Ring:
		modelData_ = CreateRingPrimitive(texturePath);
		break;
	case TinyEngine::ParticleMeshType::Cylinder:
		modelData_ = CreateCylinderPrimitive(texturePath);
		break;
	default:
		break;
	}

	// 頂点データの初期化
	CreateVertexData();

	// マテリアルの初期化
	CreateMaterialData();

	// テクスチャ読み込み
	ctx_->textureManager->LoadTexture(texturePath);

	// テクスチャ番号を取得して、メンバ変数に書き込む
	modelData_.material.textureIndex = ctx_->textureManager->GetSrvIndex(texturePath);

	// カメラをセットする
	camera_ = ctx_->particleCommon->GetDefaultCamera();

	// エミッタの初期化
	if (customEmitter) {
		emitter_ = *customEmitter;
		emitter_.transform.translate = emitterPos; // 位置だけは引数で上書き
	} else {
		InitializeEmitter(emitterPos); // デフォルトのエミッタ設定
	}

	// パーティクルに加速度を与える構造体の初期化
	InitializeAccelerationField();
}

void Particle::SetEmitterForModule(const std::string& moduleName, const Emitter& emitter) { emitters_[moduleName] = emitter; }

bool Particle::HasModuleEmitter(const std::string& moduleName) const { return emitters_.find(moduleName) != emitters_.end(); }

void Particle::InitializeEmitter(Vector3 emitterPos) {
	// 従来のデフォルト初期化
	emitter_.count = 10;
	emitter_.frequency = 0.2f;
	emitter_.frequencyTime = 0.0f;
	emitter_.transform.translate = emitterPos;
	emitter_.transform.rotate = {0.0f, 0.0f, 0.0f};
	emitter_.transform.scale = {1.0f, 1.0f, 1.0f};
}

void Particle::Update() {
	// 経過時間
	timeManager_->Update();
	float dt = timeManager_->GetDeltaTime();

	// エミッタの稼働時間を更新
	elapsedTime_ += dt;

	// 新たなパーティクルを発生させる状態かどうか
	bool isActive = true;
	if (!isLoop_ && duration_ > 0.0f && elapsedTime_ >= duration_) {
		isActive = false; // 指定した稼働時間を超えたら新規発生をストップ
	}

	// エミッタのImGui
#ifdef USE_IMGUI
	ImGui::Begin("Emitter");
	ImGui::DragFloat3("Translate", &emitter_.transform.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::End();
#endif

	// エミッタの更新とパーティクル発生
	if (isActive) {
		UpdateEmitter();
	}

	numInstance_ = 0;
	for (auto it = particles_.begin(); it != particles_.end();) {
		if (it->lifeTime <= it->currentTime) {
			it = particles_.erase(it);
			continue;
		}

		// 個々のパーティクルの更新
		UpdateParticle(it);

		if (numInstance_ < kNumMaxInstance) {
			// インスタンシング用データへ書き込み
			WriteParticleToGPU(*it, numInstance_);
			++numInstance_;
		}

		++it;
	}

	// ループ再生ではなく、新規発生も止まっており、画面上のパーティクルも全て消滅したら終了
	if (!isLoop_ && !isActive && particles_.empty()) {
		isFinished_ = true;
	}
}

void Particle::Draw() {
	if (numInstance_ <= 0) {
		return;
	}

	// 3Dオブジェクト描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	ctx_->particleCommon->DrawSettingCommon();

	auto commandList = ctx_->particleCommon->GetDxCommon()->GetCommandList();

	// wvp用のBufferの場所を設定
	commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU_);

	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定

	// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// SRVのDescriptorTableの先頭を設定。3はrootParameter[3]（Pixel用テクスチャ）である。
	commandList->SetGraphicsRootDescriptorTable(2, ctx_->textureManager->GetSrvHandleGPU(modelData_.material.textureFilePath));

	// 描画!(DrawCall/ドローコール)。
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), numInstance_, 0, 0);
}

ModelData Particle::CreatePrimitive(const std::string& texturePath) {
	ModelData modelData;

	modelData.vertices.push_back({
	    .position = {-1.0f, 1.0f, 0.0f, 1.0f},
          .texcoord = {0.0f, 0.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 左上
	modelData.vertices.push_back({
	    .position = {1.0f, 1.0f, 0.0f, 1.0f},
          .texcoord = {1.0f, 0.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 右上
	modelData.vertices.push_back({
	    .position = {-1.0f, -1.0f, 0.0f, 1.0f},
          .texcoord = {0.0f, 1.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 左下
	modelData.vertices.push_back({
	    .position = {-1.0f, -1.0f, 0.0f, 1.0f},
          .texcoord = {0.0f, 1.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 左下
	modelData.vertices.push_back({
	    .position = {1.0f, 1.0f, 0.0f, 1.0f},
          .texcoord = {1.0f, 0.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 右上
	modelData.vertices.push_back({
	    .position = {1.0f, -1.0f, 0.0f, 1.0f},
          .texcoord = {1.0f, 1.0f},
          .normal = {0.0f, 0.0f, 1.0f}
    }); // 右下

	modelData.material.textureFilePath = "resources/textures/" + texturePath;

	return modelData;
}

ModelData TinyEngine::Particle::CreateRingPrimitive(const std::string& texturePath, float innerRadius, float outerRadius, uint32_t segments) { 
	ModelData modelData;

	// 円周の分割角度
	float angleStep = 2.0f * std::numbers::pi_v<float> / static_cast<float>(segments);
	Vector3 normal = {0.0f, 0.0f, 1.0f};

	for (uint32_t i = 0; i < segments; ++i) {
		// 現在のステップと次のステップの角度
		float alpha1 = static_cast<float>(i) * angleStep;
		float alpha2 = static_cast<float>(i + 1) * angleStep;

		float cos1 = std::cos(alpha1);
		float sin1 = std::sin(alpha1);
		float cos2 = std::cos(alpha2);
		float sin2 = std::sin(alpha2);

		// 扇形の外側と内側の4頂点の座標を計算
		Vector4 pOut1 = {outerRadius * cos1, outerRadius * sin1, 0.0f, 1.0f};
		Vector4 pIn1 = {innerRadius * cos1, innerRadius * sin1, 0.0f, 1.0f};
		Vector4 pOut2 = {outerRadius * cos2, outerRadius * sin2, 0.0f, 1.0f};
		Vector4 pIn2 = {innerRadius * cos2, innerRadius * sin2, 0.0f, 1.0f};

		// UV座標の計算
		auto calcUV = [outerRadius](const Vector4& p) -> Vector2 { return {p.x / (2.0f * outerRadius) + 0.5f, -p.y / (2.0f * outerRadius) + 0.5f}; };

		Vector2 uvOut1 = calcUV(pOut1);
		Vector2 uvIn1 = calcUV(pIn1);
		Vector2 uvOut2 = calcUV(pOut2);
		Vector2 uvIn2 = calcUV(pIn2);

		// 三角形1
		modelData.vertices.push_back({.position = pOut1, .texcoord = uvOut1, .normal = normal});
		modelData.vertices.push_back({.position = pIn1, .texcoord = uvIn1, .normal = normal});
		modelData.vertices.push_back({.position = pIn2, .texcoord = uvIn2, .normal = normal});

		// 三角形2
		modelData.vertices.push_back({.position = pOut1, .texcoord = uvOut1, .normal = normal});
		modelData.vertices.push_back({.position = pIn2, .texcoord = uvIn2, .normal = normal});
		modelData.vertices.push_back({.position = pOut2, .texcoord = uvOut2, .normal = normal});
	}

	modelData.material.textureFilePath = "resources/textures/" + texturePath;

	return modelData;
}

ModelData TinyEngine::Particle::CreateCylinderPrimitive(const std::string& texturePath, float topRadius, float bottomRadius, float height, uint32_t segments) {
	ModelData modelData;

	// 円周の分割角度
	float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(segments);

	for (uint32_t index = 0; index < segments; ++index) {
		float alpha = static_cast<float>(index) * radianPerDivide;
		float alphaNext = static_cast<float>(index + 1) * radianPerDivide;

		float sin = std::sin(alpha);
		float cos = std::cos(alpha);
		float sinNext = std::sin(alphaNext);
		float cosNext = std::cos(alphaNext);

		float u = static_cast<float>(index) / static_cast<float>(segments);
		float uNext = static_cast<float>(index + 1) / static_cast<float>(segments);

		// 上面、下面のY位置
		float yTop = height;
		float yBottom = 0.0f;

		Vector3 normal = {-sin, 0.0f, cos};
		Vector3 normalNext = {-sinNext, 0.0f, cosNext};

		// 三角形1
		modelData.vertices.push_back({
		    .position = {-sin * topRadius, yTop, cos * topRadius, 1.0f},
              .texcoord = {u, 1.0f},
              .normal = normal
        });
		modelData.vertices.push_back({
		    .position = {-sinNext * topRadius, yTop, cosNext * topRadius, 1.0f},
              .texcoord = {uNext, 1.0f},
              .normal = normalNext
        });
		modelData.vertices.push_back({
		    .position = {-sin * bottomRadius, yBottom, cos * bottomRadius, 1.0f},
              .texcoord = {u, 0.0f},
              .normal = normal
        });

		// 三角形2
		modelData.vertices.push_back({
		    .position = {-sin * bottomRadius, yBottom, cos * bottomRadius, 1.0f},
              .texcoord = {u, 0.0f},
              .normal = normal
        });
		modelData.vertices.push_back({
		    .position = {-sinNext * topRadius, yTop, cosNext * topRadius, 1.0f},
              .texcoord = {uNext, 1.0f},
              .normal = normalNext
        });
		modelData.vertices.push_back({
		    .position = {-sinNext * bottomRadius, yBottom, cosNext * bottomRadius, 1.0f},
              .texcoord = {uNext, 0.0f},
              .normal = normalNext
        });
	}

	modelData.material.textureFilePath = "resources/textures/" + texturePath;
	return modelData;
}

void Particle::CreateVertexData() {
	// 頂点リソースの作成
	vertexResource_ = DirectXUtils::CreateBufferResource(ctx_->particleCommon->GetDxCommon()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	vertexResource_->Unmap(0, nullptr);
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void Particle::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = DirectXUtils::CreateBufferResource(ctx_->particleCommon->GetDxCommon()->GetDevice(), sizeof(Material));
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialResource_->Unmap(0, nullptr);
	// 三角形の色
	material_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	// uvTransformなどのデータを設定
	material_.uvTransform = MathUtility::MakeIdentity4x4();
	// discard閾値設定
	material_.alphaCutoff = 0.0f;
	// materialDataに代入
	*materialData_ = material_;
}

void Particle::CreateInstancingResource() {
	// Instancing用のTransformationMatrixリソースを作る
	instancingResource_ = DirectXUtils::CreateBufferResource(ctx_->particleCommon->GetDxCommon()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance);

	// 書き込むためのアドレス取得
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	instancingResource_->Unmap(0, nullptr);

	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		instancingData_[index].WVP = MathUtility::MakeIdentity4x4();
		instancingData_[index].World = MathUtility::MakeIdentity4x4();
		instancingData_[index].color = {1.0f, 1.0f, 1.0f, 1.0f};
	}
}

// MakeParticle をエミッタ情報を受け取るよう変更し、エミッタの scale を発生範囲（±scale）としてランダム位置を決定する
ParticleState Particle::MakeParticle(const Emitter& emitter, Vector3 translate) {
	ParticleState particle;

	// デフォルト値
	particle.transform.scale = {1.0f, 1.0f, 1.0f};
	particle.transform.rotate = {0.0f, 0.0f, 0.0f};
	particle.transform.translate = translate; // 中心位置をセット（最終的にランダムオフセットを適用）

	// 発生範囲（エミッタの scale を半幅（extent）として扱う）
	Vector3 extent = emitter.transform.scale;

	// ランダムオフセットを作る（ボックス内）
	Vector3 randomOffset = {RandomUtils::RangeFloat(-extent.x, extent.x), RandomUtils::RangeFloat(-extent.y, extent.y), RandomUtils::RangeFloat(-extent.z, extent.z)};

	// モジュールがある場合はモジュール側で初期化をまかせる
	if (module_) {
		module_->Initialize(particle, ctx_);
		// モジュールはtranslateを書き換えない想定だが、発生位置はエミッタ内ランダム位置へ設定する
		particle.transform.translate = translate + randomOffset;
		return particle;
	}

	// 基準位置を指定してランダムな範囲にパーティクルを生成する（既存ロジックをエミッタスケールに対応）
	particle.transform.translate = translate;

	particle.velocity = {RandomUtils::RangeFloat(-1, 1), RandomUtils::RangeFloat(-1, 1), RandomUtils::RangeFloat(-1, 1)};
	particle.color = {1.0f, 1.0f, 1.0f, 1.0f};
	particle.lifeTime = RandomUtils::RangeFloat(1, 3);
	particle.currentTime = 0;
	return particle;
}

Matrix4x4 Particle::CreateBillboardMatrix() {
	Matrix4x4 backToFrontMatrix = MathUtility::MakeYawRotateMatrix(std::numbers::pi_v<float>);
	Matrix4x4 billboardmatrix;

	if (isBillboard_) {
		billboardmatrix = MathUtility::Multiply(backToFrontMatrix, camera_->GetWorldMatrix());
	} else {
		billboardmatrix = MathUtility::MakeIdentity4x4();
	}

	billboardmatrix.m[3][0] = 0.0f;
	billboardmatrix.m[3][1] = 0.0f;
	billboardmatrix.m[3][2] = 0.0f;

	return billboardmatrix;
}

std::list<ParticleState> Particle::Emit(const Emitter& emitter, Vector3 translate) {
	std::list<ParticleState> particles;
	for (uint32_t count = 0; count < emitter.count; ++count) {
		particles.push_back(MakeParticle(emitter, translate));
	}

	return particles;
}

void Particle::CoordinateTransformation(std::list<ParticleState>::iterator particleIterator) {
	// ビルボードマトリックスの作成
	Matrix4x4 billboardmatrix = MathUtility::MakeIdentity4x4();

	// スケール・回転・平行移動の行列を作成
	Matrix4x4 scaleMatrix = MathUtility::MakeScaleMatrix((*particleIterator).transform.scale);

	Matrix4x4 rotateMatrix = MathUtility::Multiply(
	    MathUtility::MakePitchRotateMatrix((*particleIterator).transform.rotate.x),
	    MathUtility::Multiply(MathUtility::MakeYawRotateMatrix((*particleIterator).transform.rotate.y), MathUtility::MakeRollRotateMatrix((*particleIterator).transform.rotate.z)));

	Matrix4x4 translateMatrix = MathUtility::MakeTranslateMatrix((*particleIterator).transform.translate);

	// 行列の乗算（スケール * 回転 * ビルボード * 平行移動）
	Matrix4x4 srMatrix = MathUtility::Multiply(scaleMatrix, rotateMatrix);
	worldMatrix_ = MathUtility::Multiply(MathUtility::Multiply(srMatrix, billboardmatrix), translateMatrix);

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix_ = MathUtility::Multiply(worldMatrix_, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix_ = worldMatrix_;
	}
}

void Particle::CreateInstancingSRV(UINT srvIndex) {
	DirectXCommon* dxCommon = ctx_->particleCommon->GetDxCommon();

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kNumMaxInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	instancingSrvHandleCPU_ = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), srvIndex);
	instancingSrvHandleGPU_ = dxCommon->GetGPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), srvIndex);

	dxCommon->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU_);
}

void Particle::InitializeAccelerationField() {
	accelerationField_.acceleration = {15.0f, 0.0f, 0.0f};
	accelerationField_.area.min = {-1.0f, -1.0f, -1.0f};
	accelerationField_.area.max = {1.0f, 1.0f, 1.0f};
}

void Particle::UpdateEmitter() {
	emitter_.frequencyTime += timeManager_->GetDeltaTime();
	if (emitter_.frequency <= emitter_.frequencyTime) {
		particles_.splice(particles_.end(), Emit(emitter_, emitter_.transform.translate));
		emitter_.frequencyTime -= emitter_.frequency;
	}
}

void Particle::UpdateParticle(std::list<ParticleState>::iterator& itr) {
	if (Collision::Intersect(accelerationField_.area, itr->transform.translate)) {
		itr->velocity += accelerationField_.acceleration * timeManager_->GetDeltaTime();
	}

	itr->transform.translate += itr->velocity * timeManager_->GetDeltaTime();
	itr->currentTime += timeManager_->GetDeltaTime();
	material_.uvTransform = MathUtility::MakeTranslateMatrix({itr->uvScroll.x, itr->uvScroll.y, 0.0f});

	// モジュールの更新処理を追加
	if (module_) {
		module_->Update(*itr, timeManager_->GetDeltaTime(), ctx_);
	}

	CoordinateTransformation(itr);
}

void Particle::WriteParticleToGPU(const ParticleState& p, uint32_t index) {
	// モジュールがある場合はモジュール側のアルファを優先する
	float alpha;
	if (module_) {
		alpha = p.color.w;
	} else {
		alpha = 1.0f - (p.currentTime / p.lifeTime);
	}

	instancingData_[index].WVP = worldViewProjectionMatrix_;
	instancingData_[index].World = worldMatrix_;
	instancingData_[index].color = p.color;
	instancingData_[index].color.w = alpha;
}