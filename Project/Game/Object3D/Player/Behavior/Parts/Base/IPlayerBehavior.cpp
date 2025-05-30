#include "IPlayerBehavior.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Particle/ParticleSystem.h>

//============================================================================
//	IPlayerBehavior classMethods
//============================================================================

IPlayerBehavior::IPlayerBehavior() {

	particleSystem_ = ParticleSystem::GetInstance();
}

Quaternion IPlayerBehavior::CalRotationAxisAngle(const Vector3& rotationAngle) {

	Quaternion rotation = Quaternion::Normalize(Quaternion::
		MakeRotateAxisAngleQuaternion(Vector3(1.0f, 0.0f, 0.0f), rotationAngle.x) *
		Quaternion::
		MakeRotateAxisAngleQuaternion(Vector3(0.0f, 1.0f, 0.0f), rotationAngle.y) *
		Quaternion::
		MakeRotateAxisAngleQuaternion(Vector3(0.0f, 0.0f, 1.0f), rotationAngle.z));
	return  rotation;
}

void IPlayerBehavior::InputKey(Vector2& inputValue) {

	Input* input = Input::GetInstance();
	if (input->PushKey(DIK_W)) {

		inputValue.y += 1.0f;
	} else if (input->PushKey(DIK_S)) {

		inputValue.y -= 1.0f;
	}
	if (input->PushKey(DIK_D)) {

		inputValue.x += 1.0f;
	} else if (input->PushKey(DIK_A)) {

		inputValue.x -= 1.0f;
	}
}