#include "BodyDashBehavior.h"

//============================================================================
//	include
//============================================================================
#include <Game/Camera/FollowCamera.h>
#include <Game/Object3D/Player/Parts/Base/BasePlayerParts.h>
#include <Engine/Particle/ParticleSystem.h>

//============================================================================
//	BodyDashBehavior classMethods
//============================================================================

BodyDashBehavior::BodyDashBehavior(const Json& data,
	FollowCamera* followCamera, float startSpeed) {

	followCamera_ = nullptr;
	followCamera_ = followCamera;

	speedLerpValue_ = std::make_unique<SimpleAnimation<float>>();
	if (data.contains(dashBehaviorJsonKey_) && data[dashBehaviorJsonKey_].contains("DashSpeedLerpValue")) {

		speedLerpValue_->FromJson(data[dashBehaviorJsonKey_]["DashSpeedLerpValue"]);
		rotationLerpRate_ = JsonAdapter::GetValue<float>(data[dashBehaviorJsonKey_], "rotationLerpRate_");
	}

	// 初期速度を設定する
	speedLerpValue_->move_.start = startSpeed;
}

void BodyDashBehavior::Execute(BasePlayerParts* parts) {

	// ダッシュ更新
	UpdateDash(parts);
	// 回転、向いている方向を計算
	RotateToDirection(parts);
}

void BodyDashBehavior::UpdateDash(BasePlayerParts* parts) {

	// 補間処理はダッシュ中のみ
	if (!speedLerpValue_->IsStart()) {

		// 補間処理を開始
		speed_ = speedLerpValue_->move_.start;
		speedLerpValue_->Start();

		// particleを発生させる
		Vector3 effectPos = parts->GetTransform().translation;
		effectPos += parts->GetTransform().GetForward() * 6.4f;
		effectPos.y = 1.0f;
		particleSystem_->SetTranslate("dashEffectEmitter", effectPos);
		particleSystem_->SetRotate("dashEffectEmitter", parts->GetTransform().rotation);
		particleSystem_->Emit("dashEffectEmitter");
		emitParticle_ = true;
	}
	// 値を補間
	speedLerpValue_->LerpValue(speed_);

	Vector2 inputValue{};
	// inputの値を取得
	IPlayerBehavior::InputKey(inputValue);
	inputValue = Vector2::Normalize(inputValue);
	if (std::fabs(inputValue.x) > FLT_EPSILON || std::fabs(inputValue.y) > FLT_EPSILON) {

		// 入力がある場合のみ速度を計算する
		Vector3 inputDirection(inputValue.x, 0.0f, inputValue.y);
		inputDirection = Vector3::Normalize(inputDirection);

		Matrix4x4 rotateMatrix = Quaternion::MakeRotateMatrix(followCamera_->GetTransform().rotation);
		Vector3 rotatedDirection = Vector3::TransferNormal(inputDirection, rotateMatrix);
		rotatedDirection = Vector3::Normalize(rotatedDirection);

		move_ = rotatedDirection * speed_;
	}

	// 移動量を加算
	Vector3 translation = parts->GetTransform().translation;
	translation.x += move_.x;
	translation.z += move_.z;
	parts->SetTranslate(translation);
}

void BodyDashBehavior::RotateToDirection(BasePlayerParts* parts) {

	Vector3 direction = Vector3(move_.x, 0.0f, move_.z).Normalize();

	if (direction.Length() <= 0.0f) {
		return;
	}

	// 向きを計算
	Quaternion targetRotation = Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f));
	Quaternion rotation = parts->GetTransform().rotation;
	rotation = Quaternion::Slerp(rotation, targetRotation, rotationLerpRate_);
	parts->SetRotate(rotation);
}

void BodyDashBehavior::Reset() {

	// 初期化する
	speedLerpValue_->Reset();
}

void BodyDashBehavior::ImGui() {

	ImGui::Text(std::format("moveX: {}", move_.x).c_str());
	ImGui::Text(std::format("moveZ: {}", move_.z).c_str());

	ImGui::PushItemWidth(itemWidth_);

	ImGui::DragFloat("rotationLerpRate##BodyDashBehavior", &rotationLerpRate_, 0.01f);

	ImGui::PopItemWidth();

	speedLerpValue_->ImGui("PlayerBodyDashBehavior_speedLerpValue_");
}

void BodyDashBehavior::SaveJson(Json& data) {

	// 値の保存
	speedLerpValue_->ToJson(data[dashBehaviorJsonKey_]["DashSpeedLerpValue"]);
	data[dashBehaviorJsonKey_]["rotationLerpRate_"] = rotationLerpRate_;
}