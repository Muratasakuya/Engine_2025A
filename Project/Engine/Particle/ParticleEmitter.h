#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Particle/ParticleParameter.h>

// emitter
#include <Engine/Core/Graphics/Mesh/Mesh.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Core/ECS/Components/TransformComponent.h>

// imgui
#include <imgui.h>
// c++
#include <cstdint>
#include <list>

//============================================================================
//	structures
//============================================================================

// bufferデータ
struct EffectMaterial {

	//色
	Color color;

	// texure
	uint32_t textureIndex;
	uint32_t noiseTextureIndex;

	// flags
	int32_t useNoiseTexture; // noiseTextureを使うか
	int32_t useVertexColor;  // 頂点色を使うか

	// sampler
	int32_t samplerType; // 使用するsamplerの種類

	// alphaReference
	float textureAlphaReference;      // 貼るtexture
	float noiseTextureAlphaReference; // noiseTexture

	// emission
	float emissiveIntensity; // 発光強度
	Vector3 emissionColor;   // 発光色

	// noiseDiscard
	float edgeSize;  // edgeのサイズ
	Color edgeColor; // edgeの色
	float edgeEmissiveIntensity; // edgeの発光強度
	Vector3 edgeEmissionColor;   // edgeの発光色

	// uv
	Matrix4x4 uvTransform;

	void Init();
	void SetMaterial(const EffectMaterial& material);
};

// 各particleの情報
struct ParticleData {

	// 動かす元の値
	// groupのparameterから値を取得してくる
	ParticleParameter parameter;

	// timer(T)
	float currentTime; // 現在の経過時間
	float progress;    // currentTの値0.0f -> 1.0f

	// 移動方向(速度)
	Vector3 velocity;      // 設定速度、groupのparameterから値を取得してくる
	Vector3 easedVelocity; // easedProgressTを掛けた後の値

	// bufferに渡すデータ
	EffectMaterial material; // material
	BaseTransform transform; // transform(matrix)
};

// particleの集まり
struct ParticleGroup {

	// mesh情報
	std::unique_ptr<EffectMesh> mesh;
	// structuredBuffer
	DxConstBuffer<EffectMaterial> materialBuffer; // material
	DxConstBuffer<Matrix4x4> worldMatrixBuffer;   // matrix
	// instance数
	uint32_t numInstance;

	// transter
	std::vector<EffectMaterial> transferMaterials;
	std::vector<Matrix4x4> transferMatrices;
	// 前フレームのemitterの座標
	Vector3 preEmitterPos;

	// particleを構築するparameter
	ParticleParameter parameter;
	// particle情報
	std::list<ParticleData> particles;

	ParticleGroup() = default;
	~ParticleGroup() = default;

	// ムーブコンストラクタ
	ParticleGroup(ParticleGroup&& other) noexcept
		: mesh(std::move(other.mesh)),
		materialBuffer(std::move(other.materialBuffer)),
		worldMatrixBuffer(std::move(other.worldMatrixBuffer)),
		numInstance(other.numInstance),
		transferMaterials(std::move(other.transferMaterials)),
		transferMatrices(std::move(other.transferMatrices)),
		preEmitterPos(other.preEmitterPos),
		parameter(std::move(other.parameter)),
		particles(std::move(other.particles)) {
		other.numInstance = 0;
	}

	// ムーブ代入演算子
	ParticleGroup& operator=(ParticleGroup&& other) noexcept {
		if (this != &other) {
			mesh = std::move(other.mesh);
			materialBuffer = std::move(other.materialBuffer);
			worldMatrixBuffer = std::move(other.worldMatrixBuffer);
			numInstance = other.numInstance;
			transferMaterials = std::move(other.transferMaterials);
			transferMatrices = std::move(other.transferMatrices);
			preEmitterPos = other.preEmitterPos;
			parameter = std::move(other.parameter);
			particles = std::move(other.particles);
			other.numInstance = 0;
		}
		return *this;
	}

	// コピーは禁止
	ParticleGroup(const ParticleGroup&) = delete;
	ParticleGroup& operator=(const ParticleGroup&) = delete;
};

//============================================================================
//	ParticleEmitter class
//============================================================================
class ParticleEmitter {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleEmitter() = default;
	~ParticleEmitter() = default;

	void Init(const std::string& name, Asset* asset, ID3D12Device* device);
	void Init(const Json& data, const std::string& name, Asset* asset, ID3D12Device* device);

	void Update(const Matrix4x4& billboardMatrix, bool useGame);

	void ImGui();

	// 発生処理
	void Emit();
	// 一定間隔で発生
	void UpdateFrequencyEmit();
	//--------- accessor -----------------------------------------------------

	void SetParent(const BaseTransform& parent) { transform_.parent = &parent; }

	// 各emitterへのsetter
	// 座標
	void SetTranslate(const Vector3& translate);
	// 回転
	void SetRotate(const Quaternion& rotate);

	const std::vector<ParticleGroup>& GetParticleGroup() const { return particleGroups_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structrue ----------------------------------------------------

	struct InputTextValue {

		char nameBuffer[128]; // imgui入力用
		std::string name;     // 入力した文字を取得する用

		void Reset();
	};

	//--------- variables ----------------------------------------------------

	Asset* asset_;
	ID3D12Device* device_;

	// 最大インスタンス数
	const uint32_t kMaxInstanceNum_ = 2048;

	// emitterの情報
	// 名前
	std::string name_;

	// 各particleの親となる
	BaseTransform transform_;

	// 所持しているparticle
	std::vector<ParticleGroup> particleGroups_;

	// particle追加入力処理
	InputTextValue addParticleNameInputText_;
	std::string addModelName_;
	std::optional<int> currentSelectIndex_;

	// editor
	ImVec2 addButtonSize_;  // 追加ボタンサイズ
	ImVec2 dropSize_;       // ドロップ処理受け取りサイズ
	ImVec2 leftChildSize_;  // 左側
	ImVec2 rightChildSize_; // 右側
	bool isAllEmit_;            // 全てのparticleを同時に発生させるか
	float allEmitCurrentTimer_; // 全てのparticleを同時に発生させるためのクールタイム
	float allEmitTime_;         // 全てのparticleを同時に発生させるまでの時間

	// 保存処理
	InputTextValue emitterSave_;
	InputTextValue particleSave_;
	bool showEmitterPopup_ = false;
	bool showParticlePopup_ = false;

	//--------- functions ----------------------------------------------------

	void EditLayout();

	// 保存処理
	void SaveEmitter();

	// 全てのparticleを同時に発生させる
	void EmitAllParticle();
	void UpdateAllParticle(bool useGame);

	// 作成処理
	// editorから
	void CreateParticle();
	// jsonから
	void CreateParticle(const Json& data);
	// meshletの作成
	ResourceMesh<EffectMeshVertex> CreateMeshlet(const std::string& modelName);

	// emitter描画処理
	void DrawParticleEmitters(bool useGame);
	// 発生処理
	void UpdateFrequencyEmit(ParticleGroup& group);
	void UpdateInterpolateEmit(ParticleGroup& group);
	// 各particleを更新
	void UpdateParticles(const Matrix4x4& billboardMatrix, bool useGame);

	// 追加処理
	void AddParticle();
	// 読み込み処理
	void LoadParticle();
	// 選択処理
	void SelectParticle();
	// 値操作
	void EditParticle();
	// 保存処理
	void SaveParticle();
	// 削除処理
	void RemoveParticle();

	void SetEmitterPos(ParticleGroup& group, const Vector3& pos);
	Vector3 GetEmitterPos(const ParticleParameter& parameter) const;
};