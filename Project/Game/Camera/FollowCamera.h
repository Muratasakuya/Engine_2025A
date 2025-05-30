#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>

//============================================================================
//	FollowCamera class
//============================================================================
class FollowCamera :
	public BaseCamera {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCamera() = default;
	~FollowCamera() = default;

	void Init() override;

	void Update() override;

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	void StartScreenShake() { isScreenShake_ = true; }

	void SetTarget(const Transform3DComponent& target) { target_ = &target; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const Transform3DComponent* target_;
	float aspectRatio_;

	// parameter
	Vector3 offsetTranslation_;    // 追従相手との距離
	Vector3 interTarget_;          // 追従中間target位置
	float eulerRotateClampPlusX_;  // カメラのX軸回転の制限: +
	float eulerRotateClampMinusX_; // カメラのX軸回転の制限: -

	float lerpRate_;      // 補間速度
	Vector2 sensitivity_; // マウス感度

	bool isScreenShake_;                 // 画面シェイク中かどうか
	float screenShakeIntensity_ = 0.8f; // 画面シェイクの強度
	float screenShakeDuration_ = 0.32f; // 画面シェイクの持続時間
	float screenShakeTimer_ = 0.0f;     // シェイクの経過時間
	
	// editor
	bool isDebugMode_;

	//--------- functions ----------------------------------------------------

	// init
	void InitParam();

	// update
	void Move();
	void UpdateMatrix();

	void UpdateScreenShake();

	// json
	void ApplyJson();
	void SaveJson();
};