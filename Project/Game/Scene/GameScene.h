#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/Methods/IScene.h>
#include <Engine/Scene/Light/PunctualLight.h>

// camera
#include <Game/Camera/GameCamera.h>
#include <Game/Camera/FollowCamera.h>

// object
#include <Game/Object3D/Player/Player.h>

//============================================================================
//	GameScene class
//============================================================================
class GameScene :
	public IScene, public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameScene() :IGameEditor("GameScene") {};
	~GameScene() = default;

	void Init(Asset* asset, CameraManager* cameraManager,
		LightManager* lightManager, PostProcessSystem* postProcessSystem) override;

	void Update(SceneManager* sceneManager) override;

	void ImGui() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// camera
	std::unique_ptr<FollowCamera> followCamera_;
	// light
	std::unique_ptr<PunctualLight> gameLight_;

	// objects
	std::unique_ptr<Player> player_;

	// debug
	std::vector<uint32_t> debugId_;
	std::vector<bool> debugAnimationContoroller_;

	//--------- functions ----------------------------------------------------

	void Load(Asset* asset);

	void CreatePrimitive(Asset* asset);
};