#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/SimpleAnimation.h>
#include <Engine/Utility/KeyframeAnimation.h>
#include <Game/Object3D/Player/Behavior/PlayerBehaviorType.h>
#include <Lib/Adapter/JsonAdapter.h>

// c++
#include <memory>
#include <optional>
// front
class BasePlayerParts;
class FollowCamera;
class ParticleSystem;

//============================================================================
//	IPlayerBehavior class
//============================================================================
class IPlayerBehavior {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IPlayerBehavior();
	virtual ~IPlayerBehavior() = default;

	// 処理実行
	virtual void Execute(BasePlayerParts* parts) = 0;
	// リセット
	virtual void Reset() = 0;

	// imgui
	virtual void ImGui() = 0;

	// json
	virtual void SaveJson(Json& data) = 0;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const float itemWidth_ = 224.0f;

	FollowCamera* followCamera_;
	ParticleSystem* particleSystem_;

	// behaviorごとのjsonのkey
	const std::string waitBehaviorJsonKey_ = "BehaviorType_Wait";
	const std::string walkBehaviorJsonKey_ = "BehaviorType_Walk";
	const std::string dashBehaviorJsonKey_ = "BehaviorType_Dash";
	const std::string attack1stBehaviorJsonKey_ = "BehaviorType_Attack_1st";
	const std::string dashAttackBehaviorJsonKey_ = "BehaviorType_DashAttack";
	const std::string attack2ndBehaviorJsonKey_ = "BehaviorType_Attack_2nd";
	const std::string attack3rdBehaviorJsonKey_ = "BehaviorType_Attack_3rd";
	const std::string parryBehaviorJsonKey_ = "BehaviorType_Parry";

	bool emitParticle_ = false;

	//--------- functions ----------------------------------------------------

	Quaternion CalRotationAxisAngle(const Vector3& rotationAngle);

	void InputKey(Vector2& inputValue);
};