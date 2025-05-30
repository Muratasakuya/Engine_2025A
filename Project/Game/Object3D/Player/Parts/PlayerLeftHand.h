#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Object3D/Player/Parts/Base/BasePlayerParts.h>

//============================================================================
//	PlayerLeftHand class
//============================================================================
class PlayerLeftHand :
	public BasePlayerParts {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerLeftHand() = default;
	~PlayerLeftHand() = default;

	void Init();

	void ImGui();

	// json
	void SaveJson();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();

	// init
	void InitBehaviors(const Json& data);
};