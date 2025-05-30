#include "GameScene.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Core/Graphics/PostProcess/PostProcessSystem.h>
#include <Engine/Core/Graphics/Skybox/Skybox.h>
#include <Engine/Scene/Camera/CameraManager.h>
#include <Engine/Scene/Light/LightManager.h>
#include <Engine/Renderer/LineRenderer.h>
#include <Engine/Core/ECS/Core/ECSManager.h>
#include <Engine/Core/ECS/Components/AnimationComponent.h>
#include <Engine/Particle/ParticleSystem.h>

//============================================================================
//	TitleScene classMethods
//============================================================================

void GameScene::Load(Asset* asset) {

	// particleTexture
	asset->LoadTexture("circle");
	asset->LoadTexture("redCircle");
	asset->LoadTexture("white");
	asset->LoadTexture("monsterBall");
	asset->LoadTexture("noise");
	asset->LoadTexture("noise1");
	asset->LoadTexture("noise2");
	asset->LoadTexture("smallCircle");
	asset->LoadTexture("spark");
	asset->LoadTexture("gradationLine_1");
	asset->LoadTexture("gradationLine_0");
	asset->LoadTexture("effectCircle");
	asset->LoadTexture("flashLlight");
	asset->LoadTexture("groundEffect");
	asset->LoadTexture("smoke");
	asset->LoadTexture("electric");
	asset->LoadTexture("electric1");
	asset->LoadTexture("electric2");
	asset->LoadTexture("electric3");
	asset->LoadTexture("starCircle");
	asset->LoadTexture("starCircle2");
	// cubeMap、.dds
	asset->LoadTexture("docklands_01_2k");

	// particleModel
	asset->LoadModel("cube");
	asset->LoadModel("axis");
	asset->LoadModel("billboardPlane");
	asset->LoadModel("topLocalbillboardPlane");
	asset->LoadModel("bottomLocalbillboardPlane");
	asset->LoadModel("groundPlane");
	asset->LoadModel("crossPlane");
	asset->LoadModel("hitLine");
	asset->LoadModel("groundRing");
	asset->LoadModel("verticalRing");
	asset->LoadModel("verticalPlane");

	// player
	asset->LoadModel("playerBody");
	asset->LoadModel("playerLeftHand");
	asset->LoadModel("playerRightHand");
	asset->LoadModel("playerSword");

	// environment
	asset->LoadModel("stageField");
}

void GameScene::CreatePrimitive(Asset* asset) {

	// Ringを作成してexportする
	{
		const uint32_t kRingDivide = 32;
		const float kOuterRadius = 0.4f;
		const float kInnerRadius = 0.8f;
		const float radianPerDivide = 2.0f * pi / static_cast<float>(kRingDivide);

		std::vector<MeshVertex> vertices;
		vertices.reserve(kRingDivide * 4);

		for (uint32_t index = 0; index < kRingDivide; ++index) {

			// 今の角度と次の角度
			float sin0 = std::sin(index * radianPerDivide);
			float cos0 = std::cos(index * radianPerDivide);
			float sin1 = std::sin((index + 1) * radianPerDivide);
			float cos1 = std::cos((index + 1) * radianPerDivide);

			// UV座標
			float u0 = static_cast<float>(index) / static_cast<float>(kRingDivide);
			float u1 = static_cast<float>(index + 1U) / static_cast<float>(kRingDivide);

			// 外周‐先頭
			vertices.push_back({
				Vector4(-sin0 * kOuterRadius,  cos0 * kOuterRadius, 0.0f, 1.0f),
				Vector2(u0, 0.0f),
				Vector3(0.0f, 0.0f, 1.0f) });

			// 外周‐次点
			vertices.push_back({
				Vector4(-sin1 * kOuterRadius,  cos1 * kOuterRadius, 0.0f, 1.0f),
				Vector2(u1, 0.0f),
				Vector3(0.0f, 0.0f, 1.0f) });

			// 内周‐先頭
			vertices.push_back({
				Vector4(-sin0 * kInnerRadius,  cos0 * kInnerRadius, 0.0f, 1.0f),
				Vector2(u0, 1.0f),
				Vector3(0.0f, 0.0f, 1.0f) });

			// 内周‐次点
			vertices.push_back({
				Vector4(-sin1 * kInnerRadius,  cos1 * kInnerRadius, 0.0f, 1.0f),
				Vector2(u1, 1.0f),
				Vector3(0.0f, 0.0f, 1.0f) });
		}

		std::vector<uint32_t> indices;
		indices.reserve(kRingDivide * 6);

		for (uint32_t i = 0; i < kRingDivide; ++i) {
			uint32_t o = i * 4;

			// 三角形0
			indices.push_back(o + 0);
			indices.push_back(o + 1);
			indices.push_back(o + 3);

			// 三角形1
			indices.push_back(o + 0);
			indices.push_back(o + 3);
			indices.push_back(o + 2);
		}

		asset->Export(vertices, indices, "OutputPrimitive/primitiveRing.obj");
		asset->MakeModel("primitiveRing", vertices, indices);
	}

	// Cylinderを作成してexport する
	{
		const uint32_t kCylinderDivide = 32; // 分割数
		const float    kTopRadius = 1.0f;    // 上面半径
		const float    kBottomRadius = 1.0f; // 下面半径
		const float    kHeight = 3.0f;       // 円筒の高さ
		const float    radianPerDivide = 2.0f * pi / static_cast<float>(kCylinderDivide);

		// 頂点生成
		std::vector<MeshVertex> vertices;
		vertices.reserve(kCylinderDivide * 4);
		for (uint32_t index = 0; index < kCylinderDivide; ++index) {

			// 今の角度と次の角度
			float sin0 = std::sin(index * radianPerDivide);
			float cos0 = std::cos(index * radianPerDivide);
			float sin1 = std::sin((index + 1) * radianPerDivide);
			float cos1 = std::cos((index + 1) * radianPerDivide);

			// UV 座標
			float u0 = static_cast<float>(index) / static_cast<float>(kCylinderDivide);
			float u1 = static_cast<float>(index + 1U) / static_cast<float>(kCylinderDivide);

			// 上面・先頭
			vertices.push_back({
				Vector4(-sin0 * kTopRadius,  kHeight, cos0 * kTopRadius, 1.0f), // pos
				Vector2(u0, 0.0f),                                              // uv (上=0)
				Vector3(-sin0, 0.0f,  cos0) });                                 // normal

			// 上面・次点
			vertices.push_back({
				Vector4(-sin1 * kTopRadius,  kHeight, cos1 * kTopRadius, 1.0f),
				Vector2(u1, 0.0f),
				Vector3(-sin1, 0.0f,  cos1) });

			// 下面・先頭
			vertices.push_back({
				Vector4(-sin0 * kBottomRadius, 0.0f, cos0 * kBottomRadius, 1.0f),
				Vector2(u0, 1.0f),
				Vector3(-sin0, 0.0f,  cos0) });

			// 下面・次点
			vertices.push_back({
				Vector4(-sin1 * kBottomRadius, 0.0f, cos1 * kBottomRadius, 1.0f),
				Vector2(u1, 1.0f),
				Vector3(-sin1, 0.0f,  cos1) });
		}

		// インデックス生成
		std::vector<uint32_t> indices;
		indices.reserve(kCylinderDivide * 6);
		for (uint32_t i = 0; i < kCylinderDivide; ++i) {
			uint32_t o = i * 4;

			// 三角形0
			indices.push_back(o + 0);
			indices.push_back(o + 1);
			indices.push_back(o + 3);

			// 三角形1
			indices.push_back(o + 0);
			indices.push_back(o + 3);
			indices.push_back(o + 2);
		}

		asset->Export(vertices, indices, "OutputPrimitive/primitiveCylinder.obj");
		asset->MakeModel("primitiveCylinder", vertices, indices);
	}
}

void GameScene::Init(
	[[maybe_unused]] Asset* asset,
	[[maybe_unused]] CameraManager* cameraManager,
	[[maybe_unused]] LightManager* lightManager,
	[[maybe_unused]] PostProcessSystem* postProcessSystem
) {

	//========================================================================
	//	load
	//========================================================================

	Load(asset);

	// primitive作成
	CreatePrimitive(asset);

	// particle
	ParticleSystem::GetInstance()->LoadEmitter("fieldEffectEmitter", "fieldEffectEmitter");
	ParticleSystem::GetInstance()->LoadEmitter("hitEffectEmitter", "hitEffectEmitter");
	ParticleSystem::GetInstance()->LoadEmitter("dashEffectEmitter", "dashEffectEmitter");
	ParticleSystem::GetInstance()->LoadEmitter("electricEffectEmitter", "electricEffectEmitter");
	ParticleSystem::GetInstance()->LoadEmitter("verticalElectricEffectEmitter", "verticalElectricEffectEmitter");
	ParticleSystem::GetInstance()->LoadEmitter("attackFieldEmitter", "attackFieldEmitter");

	//========================================================================
	//	postProcess
	//========================================================================

	postProcessSystem_ = postProcessSystem;
	postProcessSystem_->Create({ PostProcessType::Bloom });

	postProcessSystem_->AddProcess(PostProcessType::Bloom);

	//========================================================================
	//	sceneObject
	//========================================================================

	// camera
	followCamera_ = std::make_unique<FollowCamera>();
	followCamera_->Init();

	cameraManager->SetCamera(followCamera_.get());

	// light
	gameLight_ = std::make_unique<PunctualLight>();
	gameLight_->Init();

	lightManager->SetLight(gameLight_.get());

	//========================================================================
	//	initObject
	//========================================================================

	player_ = std::make_unique<Player>();
	player_->Init(followCamera_.get());

	// 追従先を設定する: player
	followCamera_->SetTarget(player_->GetTransform());

	// skybox
	Skybox::GetInstance()->Create(asset->GetTextureGPUIndex("docklands_01_2k"));

	// 仮の地面
	uint32_t id = ECSManager::GetInstance()->CreateObject3D("stageField", "field", "Environment");
	auto transform = ECSManager::GetInstance()->GetComponent<Transform3DComponent>(id);
	transform->scale.x = 128.0f;
	transform->scale.z = 128.0f;
	auto material = ECSManager::GetInstance()->GetComponent<MaterialComponent, true>(id);
	material->front().material.uvTransform = Matrix4x4::MakeAffineMatrix(Vector3(24.0f, 24.0f, 0.0f),
		Vector3::AnyInit(0.0f), Vector3::AnyInit(0.0f));
	material->front().material.shadowRate = 1.0f;

	id = ECSManager::GetInstance()->CreateObject3D("cube", "debugEnemy", "Boss");
	auto enemyTransform = ECSManager::GetInstance()->GetComponent<Transform3DComponent>(id);
	enemyTransform->translation.y = 2.0f;
	enemyTransform->scale = Vector3::AnyInit(2.0f);

	auto enemyMaterial = ECSManager::GetInstance()->GetComponent<MaterialComponent, true>(id);
	enemyMaterial->front().material.color = Color(0.8f, 0.8f, 0.8f, 0.8f);
}

void GameScene::Update([[maybe_unused]] SceneManager* sceneManager) {

	followCamera_->Update();

	player_->Update();

	// particleの更新処理
	ParticleSystem::GetInstance()->FrequencyEmit("fieldEffectEmitter");
	ParticleSystem::GetInstance()->UpdateEmitter("fieldEffectEmitter");
}

void GameScene::ImGui() {
}