#pragma once
#include "AABB.h"
#include "DebugCamera.h"
#include "EngineContext.h"
#include "ModelData.h"
#include "ParticleForGPU.h"
#include "ParticleModule.h"
#include "ParticleState.h"
#include "TimeManager.h"
#include "Transform.h"
#include "VertexData.h"
#include "modules/ParticleModuleHeader.h"
#include <d3d12.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <wrl.h>

struct ParticleMaterial {
	Vector4 color;
	Matrix4x4 uvTransform;
	float alphaCutoff;
	float padding[3];
};

namespace TinyEngine {
enum class ParticleMeshType {
	Square,   // 従来の矩形
	Ring,     // リング
	Cylinder, // シリンダー
};

/// <summary>
/// パーティクルクラス
/// </summary>
class Particle {
public:
	struct Emitter {
		Transform transform; // エミッタのトランスフォーム
		uint32_t count;      // 発生数
		float frequency;     // 発生頻度
		float frequencyTime; // 頻度用時刻
	};

private:
	struct AccelerationField {
		Vector3 acceleration; // 加速度
		AABB area;            // 範囲
	};

public:
	/// <summary>
	///	初期化処理
	/// </summary>
	/// <param name="ctx">エンジンコンテキスト</param>
	/// <param name="emitterPos">エミッタの位置</param>
	/// <param name="texturePath">テクスチャのパス</param>
	/// <param name="module">適用する挙動モジュール（nullptrでデフォルト）</param>
	/// <param name="customEmitter">カスタムエミッタ設定（nullptrでデフォルト）</param>
	void Initialize(
	    EngineContext* ctx, Vector3 emitterPos, const std::string& texturePath, std::unique_ptr<ParticleModule> module = nullptr, const Emitter* customEmitter = nullptr,
	    ParticleMeshType meshType = ParticleMeshType::Ring);

	/// <summary>
	/// 更新関数
	/// </summary>
	void Update();

	/// <summary>
	/// 描画関数
	/// </summary>
	void Draw();

	// setter
	void SetColor(Vector4 color) { materialData_->color = color; }
	void SetWorldMatrix(Matrix4x4 worldMatrix) { worldMatrix_ = worldMatrix; }
	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetIsBillboard(bool isBillboard) { isBillboard_ = isBillboard; }
	void SetTranslate(Vector3 translate) { emitter.transform.translate = translate; }
	void SetEmitMode(bool isLoop, float duration = 0.0f) {
		isLoop_ = isLoop;
		duration_ = duration;
	}
	void SetEmitterParam(uint32_t count, float frequency) {
		emitter.count = count;
		emitter.frequency = frequency;
		emitter.frequencyTime = frequency;
	}

	// モジュールごとのエミッタを登録する
	void SetEmitterForModule(const std::string& moduleName, const Emitter& emitter);
	bool HasModuleEmitter(const std::string& moduleName) const;

	// discard閾値設定
	void SetAlphaCutoff(float cutoff) {
		material_.alphaCutoff = cutoff;
		materialData_->alphaCutoff = cutoff;
	}

	// getter
	Vector4& GetColor() { return material_.color; }
	Matrix4x4& GetWorldMatrix() { return worldMatrix_; }
	ParticleMaterial& GetMaterial() { return material_; }
	bool IsFinished() const { return isFinished_; }

private:
	// 四角形の作成
	ModelData CreatePrimitive(const std::string& texturePath);

	// リング状モデルの作成
	ModelData CreateRingPrimitive(const std::string& texturePath, float innerRadius = 0.2f, float outerRadius = 1.0f, uint32_t segments = 32);

	// シリンダーモデルの作成
	ModelData CreateCylinderPrimitive(const std::string& texturePath, float topRadius = 1.0f, float bottomRadius = 1.0f, float height = 3.0f, uint32_t segments = 32);

	// 頂点データの初期化
	void CreateVertexData();

	// マテリアルの初期化
	void CreateMaterialData();

	// instancingResourceの作成
	void CreateInstancingResource();

	// パーティクル生成関数
	ParticleState MakeParticle(const Emitter& emitter, Vector3 translate);

	// BillboardMatrixを作成する
	Matrix4x4 CreateBillboardMatrix();

	// エミッターを使ってパーティクルの生成
	std::list<ParticleState> Emit(const Emitter& emitter, Vector3 translate);

	// 座標変換
	void CoordinateTransformation(std::list<ParticleState>::iterator particleIterator);

	// SRV作成
	void CreateInstancingSRV(UINT srvIndex);

	// エミッタの初期化（モジュール名指定オーバーロードあり）
	void InitializeEmitter(Vector3 emitterPos);

	// 加速度フィールドの初期化
	void InitializeAccelerationField();

	// エミッタからパーティクルを生成する処理
	void UpdateEmitter();

	// パーティクル1つ更新
	void UpdateParticle(std::list<ParticleState>::iterator& itr);

	// GPU転送データへ書き込み
	void WriteParticleToGPU(const ParticleState& p, uint32_t index);

private:
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	// データ変更用の変数
	ParticleMaterial material_;
	Matrix4x4 worldMatrix_;

	// 描画する数
	static const uint32_t kNumMaxInstance = 100;
	uint32_t numInstance_ = 0;

	// 3Dオブジェクト自体とカメラの座標変換行列の元となるTransform
	std::list<ParticleState> particles_;

	Matrix4x4 worldViewProjectionMatrix_;

	// カメラ
	Camera* camera_ = nullptr;

	// モデルデータ
	ModelData modelData_;

	// VertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	// Resource
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	// Resourceにデータを書き込むためのポインタ
	VertexData* vertexData_ = nullptr;
	ParticleMaterial* materialData_ = nullptr;
	ParticleForGPU* instancingData_ = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_;

	// ビルボードするかどうか
	bool isBillboard_ = true;

	// デフォルトエミッタ
	Emitter emitter{};

	// モジュールごとのエミッタ設定
	std::unordered_map<std::string, Emitter> emitters_;

	// パーティクルに加速度を与える範囲
	AccelerationField accelerationField_;

	// 経過時間
	std::unique_ptr<TimeManager> timeManager_ = std::make_unique<TimeManager>();

	// コンテキスト構造体
	EngineContext* ctx_;

	// 挙動モジュール
	std::unique_ptr<ParticleModule> module_;

	// エミッタの寿命管理用パラメータ
	float duration_ = 0.0f;    // エミッタの稼働時間
	float elapsedTime_ = 0.0f; // エミッタの経過時間
	bool isLoop_ = true;       // ループするかどうか
	bool isFinished_ = false;  // すべての処理が終わり、削除可能かどうか
};
} // namespace TinyEngine