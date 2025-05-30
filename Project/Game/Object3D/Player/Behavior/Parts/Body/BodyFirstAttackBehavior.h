#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Object3D/Player/Behavior/Parts/Base/IPlayerBehavior.h>

//============================================================================
//	BodyFirstAttackBehavior class
//============================================================================
class BodyFirstAttackBehavior :
	public IPlayerBehavior {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BodyFirstAttackBehavior(const Json& data);
	~BodyFirstAttackBehavior() = default;

	// 処理実行
	void Execute(BasePlayerParts* parts) override;
	// リセット
	void Reset();

	// imgui
	void ImGui() override;

	// json
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// parameter
	// 進む量
	float moveValue_;
	// 向いている方向に向かって直進させる
	std::unique_ptr<SimpleAnimation<Vector3>> moveStraight_;
	// 回転角
	Vector3 rotationAngle_;
	// 開始時の回転角
	std::optional<Quaternion> startRotation_;
	// 左後ろに回転(下にかがみながら)
	std::unique_ptr<SimpleAnimation<Vector3>> backLeftRotation_;
	// 正面に向かせる(右重心にできるといい)
	std::unique_ptr<SimpleAnimation<Vector3>> forwardRightShift_;

	// particle
	Vector3 startDirection_;

	//--------- functions ----------------------------------------------------

	void UpdateMoveStraight(BasePlayerParts* parts);
	void UpdateBackLeftRotation(BasePlayerParts* parts);
	void UpdateForwardRightShift(BasePlayerParts* parts);
};