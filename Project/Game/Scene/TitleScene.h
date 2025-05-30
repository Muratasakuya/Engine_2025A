#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/Methods/IScene.h>
#include <Game/Camera/GameCamera.h>

// object
#include <Game/Object2D/SceneTransition/FadeTransition.h>

//============================================================================
//	TitleScene class
//============================================================================
class TitleScene :
	public IScene {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	TitleScene() = default;
	~TitleScene() = default;

	void Init(Asset* asset, CameraManager* cameraManager,
		LightManager* lightManager, PostProcessSystem* postProcessSystem) override;

	void Update(SceneManager* sceneManager) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::unique_ptr<GameCamera> gameCamera_;

	uint32_t titleNameId_;
};