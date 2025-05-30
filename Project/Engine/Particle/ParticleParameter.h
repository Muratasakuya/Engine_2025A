#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Lib/DxStructures.h>
#include <Engine/Particle/Structures/EmitterStructures.h>
#include <Engine/Particle/Structures/ParticleValue.h>
#include <Lib/Adapter/Easing.h>

// front
class Asset;

//============================================================================
//	ParticleParameter class
//============================================================================
class ParticleParameter {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	//--------- functions ----------------------------------------------------

	// 初期値設定
	// editorから
	void Init(std::string name, const std::string& modelName, Asset* asset);
	// jsonから
	void Init(const Json& data, Asset* asset);

	void ImGui();

	// 保存操作
	void SaveJson(const std::string& saveName);

	//--------- variables ----------------------------------------------------

	// 描画ブレンドモード
	BlendMode blendMode;

	// emitterの形状
	EmitterShapeType emitterShape;
	// emitterの形状データ
	EmitterSphere emitterSphere;
	EmitterHemisphere emitterHemisphere;
	EmitterBox emitterBox;
	EmitterCone emitterCone;

	// billboardの種類
	ParticleBillboardType billboardType;

	// samplerの種類、設定
	UVAddressMode uvAdressMode;

	// parameter
	ParticleValue<uint32_t> emitCount; // 1度に発生させる数
	float frequency;                   // ~秒置き、発生頻度
	float frequencyTime;               // 発生頻度用の現在の時刻
	ParticleValue<float> lifeTime;     // 寿命

	EasingType moveEasingType;      // 移動に使うイージング
	ParticleValue<float> moveSpeed; // 移動速度

	EasingType scaleEasingType;         // スケールに使うイージング
	ParticleValue<Vector3> startScale;  // 開始スケール
	ParticleValue<Vector3> targetScale; // 目標スケール

	EasingType rotationEasingType;                   // 回転に使うイージング
	ParticleValue<Vector3> startRotationMultiplier;  // 開始回転倍率
	ParticleValue<Vector3> targetRotationMultiplier; // 目標回転倍率

	EasingType colorEasingType;       // 色に使うイージング
	ParticleValue<Color> startColor;  // 開始色
	ParticleValue<Color> targetColor; // 目標色

	EasingType emissionEasingType;                // 発光に使うイージング
	ParticleValue<float> startEmissiveIntensity;  // 開始発光強度
	ParticleValue<float> targetEmissiveIntensity; // 目標発光強度
	ParticleValue<Vector3> startEmissionColor;    // 開始発光色
	ParticleValue<Vector3> targetEmissionColor;   // 目標発光色

	EasingType alphaReferenceEasingType;              // alphaDiscardに使うイージング
	ParticleValue<float> startTextureAlphaReference;  // 貼るtexture、開始alphaReference
	ParticleValue<float> targetTextureAlphaReference; // 貼るtexture、目標alphaReference

	ParticleValue<float> startNoiseTextureAlphaReference;  // noiseTexture、開始alphaReference
	ParticleValue<float> targetNoiseTextureAlphaReference; // noiseTexture、目標alphaReference

	EasingType edgeEasingType;            // エッジに使うイージング
	ParticleValue<float> startEdgeSize;   // 開始エッジサイズ
	ParticleValue<float> targetEdgeSize;  // 目標エッジサイズ
	ParticleValue<Color> startEdgeColor;  // 開始エッジ色
	ParticleValue<Color> targetEdgeColor; // 開始エッジ色

	EasingType edgeEmissionEasingType;                // エッジ発光に使うイージング
	ParticleValue<float> startEdgeEmissiveIntensity;  // 開始エッジ発光強度
	ParticleValue<float> targetEdgeEmissiveIntensity; // 目標エッジ発光強度
	ParticleValue<Vector3> startEdgeEmissionColor;    // 開始エッジ発光色
	ParticleValue<Vector3> targetEdgeEmissionColor;   // 目標エッジ発光色

	EasingType uvScaleEasingType;               // UVスケールに使うイージング
	ParticleValue<Vector3> startUVScale;        // 開始UVスケール
	ParticleValue<Vector3> targetUVScale;       // 目標UVスケール
	EasingType uvRotationZEasingType;           // UVZ回転に使うイージング
	ParticleValue<float> startUVRotationZ;      // 開始UVZ回転
	ParticleValue<float> targetUVRotationZ;     // 目標UVZ回転
	EasingType uvTranslationEasingType;         // UVスクロールに使うイージング
	ParticleValue<Vector3> startUVTranslation;  // 開始UV座標
	ParticleValue<Vector3> targetUVTranslation; // 目標UV座標

	Vector3 reflectFace;                     // 反射する面
	ParticleValue<float> restitution;        // 反射率.1.0fで減衰しない
	ParticleValue<float> gravityStrength;    // 重力の強さ
	ParticleValue<Vector3> gravityDirection; // 重力のかかる方向

	// texture
	uint32_t textureIndex;      // 貼るtexture
	uint32_t noiseTextureIndex; // noiseTexture

	// flags
	bool isLoop;          // ループするか
	bool interpolateEmit; // particle間の補間を行うか
	bool useScaledTime;   // スケール時間を使用するか
	bool moveToDirection; // 進行方向に移動するか
	bool reflectGround;   // 地面に反射するか
	bool useNoiseTexture; // noiseTextureを使うか
	bool useUVTransform;  // uvTransform値を使うか

	//--------- accessor -----------------------------------------------------

	const std::string& GetParticleName() const { return name_; }

	bool IsUseGame() const { return useGame_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	float itemWidth_ = 200.0f;

	Asset* asset_;

	// game内で使うかどうか
	bool useGame_;

	// particleの名前
	std::string name_;
	// modelの名前
	std::string modelName_;

	// textureの名前の保持
	std::string textureName_;
	std::string noiseTextureName_;

	//--------- functions ----------------------------------------------------

	void ImageButtonWithLabel(
		const char* id,           // ImGuiで重複しないID
		const std::string& label, // 表示したいテクスチャ名
		ImTextureID textureId,    // GPU ハンドル
		const ImVec2& size);      // 画像サイズ

	// 描画関係の値操作
	void EditRender();
	// 生成処理の値操作
	void EditEmit();
	// SRT関係の値操作
	void EditTransform();
	// 移動処理の値操作
	void EditMove();
	// material関係の値操作
	void EditMaterial();
	// physics関係の値操作
	void EditPhysics();
};