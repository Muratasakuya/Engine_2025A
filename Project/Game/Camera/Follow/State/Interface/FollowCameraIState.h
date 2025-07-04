#pragma once

//============================================================================
//	include
//============================================================================
#include <Lib/MathUtils/MathUtils.h>

// front
class FollowCamera;
class FollowCameraInputMapper;
class Transform3DComponent;

//============================================================================
//	FollowCameraIState class
//============================================================================
class FollowCameraIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraIState() = default;
	virtual ~FollowCameraIState() = default;

	// 状態遷移時
	virtual void Enter() = 0;

	// 更新処理
	virtual void Update(FollowCamera& followCamera) = 0;

	// 状態終了時
	virtual void Exit() = 0;

	// imgui
	virtual void ImGui(const FollowCamera& followCamera) = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	void SetInputMapper(const FollowCameraInputMapper* inputMapper) { inputMapper_ = inputMapper; }
	void SetTarget(const Transform3DComponent& target) { target_ = &target; }
	void SetCanExit(bool canExit) { canExit_ = canExit; }

	virtual bool GetCanExit() const { return canExit_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const FollowCameraInputMapper* inputMapper_;
	const Transform3DComponent* target_;

	// 共通parameters
	bool canExit_ = true; // 遷移可能かどうか
};