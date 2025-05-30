#pragma once

//============================================================================
//	include
//============================================================================
#include <Lib/Adapter/Easing.h>

//============================================================================
//	StateTimer class
//============================================================================
class StateTimer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	StateTimer() = default;
	~StateTimer() = default;

	void Update();

	void Reset();

	bool IsReached() const;

	float current_; // 現在の時間
	float target_;  // 目標時間
	float t_;       // イージングされていない補間T
	float easedT_;  // イージングした補間T

	EasingType easeingType_;
};