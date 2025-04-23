#include "FollowCamera.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Window/WinApp.h>
#include <Lib/Adapter/JsonAdapter.h>

//============================================================================
//	FollowCamera classMethods
//============================================================================

void FollowCamera::InitParam() {

	// アスペクト比
	aspectRatio_ = static_cast<float>(WinApp::GetWindowWidth()) /
		static_cast<float>(WinApp::GetWindowHeight());

	// transformを一回初期化
	transform_.Init();

	// json適応
	ApplyJson();
}

void FollowCamera::Init() {

	// 初期値設定
	InitParam();

	// 行列更新
	UpdateMatrix();
}

void FollowCamera::Update() {

	// 追従処理
	Move();

	// 行列更新
	UpdateMatrix();
}

void FollowCamera::Move() {

	Vector3 rotate{};
	rotate.Init();

	interTarget_ = Vector3::Lerp(interTarget_, target_->GetWorldPos(), lerpRate_);

	Vector3 offset{};
	offset.Init();

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(eulerRotation_);
	offset = Vector3::TransferNormal(offsetTranslation_, rotateMatrix);

	// offset分座標をずらす
	transform_.translation = interTarget_ + offset;
}

void FollowCamera::UpdateMatrix() {

	// eulerを設定して更新する
	transform_.rotation = Quaternion::EulerToQuaternion(eulerRotation_);

	// 行列更新
	transform_.UpdateMatrix();
	viewMatrix_ = Matrix4x4::Inverse(transform_.matrix.world);

	projectionMatrix_ =
		Matrix4x4::MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);

	viewProjectionMatrix_ = viewMatrix_ * projectionMatrix_;

	// billboardMatrixを計算
	BaseCamera::CalBillboardMatrix();
}

void FollowCamera::ImGui() {

	ImGui::PushItemWidth(itemWidth_);

	if (ImGui::Button("Save Json", ImVec2(itemWidth_, 32.0f))) {

		SaveJson();
	}

	ImGui::SeparatorText("MainParam");

	ImGui::DragFloat3("translation##FollowCamera", &transform_.translation.x, 0.01f);
	ImGui::DragFloat3("eulerRotation##FollowCamera", &eulerRotation_.x, 0.01f);
	ImGui::Text("quaternion(%4.3f, %4.3f, %4.3f, %4.3f)",
		transform_.rotation.x, transform_.rotation.y, transform_.rotation.z, transform_.rotation.w);

	ImGui::DragFloat("fovY##FollowCamera", &fovY_, 0.01f);
	ImGui::DragFloat("nearClip##FollowCamera", &nearClip_, 1.0f);
	ImGui::DragFloat("farClip##FollowCamera", &farClip_, 1.0f);

	ImGui::SeparatorText("FollowParam");

	ImGui::DragFloat3("offsetTranslation", &offsetTranslation_.x, 0.1f);
	ImGui::DragFloat("lerpRate", &lerpRate_, 0.1f);

	ImGui::PopItemWidth();
}

void FollowCamera::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/followCamera.json", data)) {
		return;
	}

	fovY_ = JsonAdapter::GetValue<float>(data, "fovY_");
	nearClip_ = JsonAdapter::GetValue<float>(data, "nearClip_");
	farClip_ = JsonAdapter::GetValue<float>(data, "farClip_");
	offsetTranslation_ = JsonAdapter::ToObject<Vector3>(data["offsetTranslation_"]);
	eulerRotation_ = JsonAdapter::ToObject<Vector3>(data["eulerRotation_"]);
	lerpRate_ = JsonAdapter::GetValue<float>(data, "lerpRate_");
}

void FollowCamera::SaveJson() {

	Json data;

	data["fovY_"] = fovY_;
	data["nearClip_"] = nearClip_;
	data["farClip_"] = farClip_;
	data["offsetTranslation_"] = JsonAdapter::FromObject<Vector3>(offsetTranslation_);
	data["eulerRotation_"] = JsonAdapter::FromObject<Vector3>(eulerRotation_);
	data["lerpRate_"] = lerpRate_;

	JsonAdapter::Save("Camera/followCamera.json", data);
}