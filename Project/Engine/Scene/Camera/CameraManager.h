#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Engine/Scene/Camera/DebugCamera.h>
#include <Engine/Scene/Camera/LightViewCamera.h>
#include <Engine/Scene/Camera/Camera2D.h>

// c++
#include <memory>
#include <optional>

//============================================================================
//	CameraManager class
//============================================================================
class CameraManager :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	CameraManager() : IGameEditor("cameraManager") {};
	~CameraManager() = default;

	void Init();

	void Update();

	void ImGui() override;
	//--------- accessor -----------------------------------------------------

	void SetCamera(BaseCamera* gameCamera);

	BaseCamera* GetCamera() const;

	DebugCamera* GetDebugCamera() const { return debugCamera_.get(); }

	LightViewCamera* GetLightViewCamera() const { return lightViewCamera_.get(); }

	Camera2D* GetCamera2D() const { return camera2D_.get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::optional<BaseCamera*> gameCamera_;

	std::unique_ptr<DebugCamera> debugCamera_;

	std::unique_ptr<LightViewCamera> lightViewCamera_;

	std::unique_ptr<Camera2D> camera2D_;

	//--------- functions ----------------------------------------------------

	void RenderCameraFrame();
};