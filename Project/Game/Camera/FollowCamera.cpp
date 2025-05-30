#include "FollowCamera.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Config.h>
#include <Game/Time/GameTimer.h>
#include <Lib/Adapter/JsonAdapter.h>
#include <Lib/Adapter/RandomGenerator.h>

//============================================================================
//	FollowCamera classMethods
//============================================================================

void FollowCamera::InitParam() {

	// アスペクト比
	aspectRatio_ = Config::kWindowWidthf / Config::kWindowHeightf;

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

	// 画面シェイク処理
	UpdateScreenShake();

	// 行列更新
	UpdateMatrix();
}

void FollowCamera::Move() {

#ifdef _DEBUG

	ImGui::Begin("Game", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove);

	bool isActive = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	ImGui::End();

	// ウィンドウを触っていない時は操作不可
	if (!isActive) {
		return;
	}
#endif

	Vector3 rotate{};
	rotate.Init();

	interTarget_ = Vector3::Lerp(interTarget_, target_->GetWorldPos(), lerpRate_);

	Vector3 offset{};
	offset.Init();

	// マウス移動量の取得
	Vector2 mouseDelta = Input::GetInstance()->GetMouseMoveValue();

	if (Input::GetInstance()->PushKey(DIK_LCONTROL)) {
		if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {

			isDebugMode_ = !isDebugMode_;
		}
	}

	if (!isDebugMode_) {

		// Y軸回転: 左右
		eulerRotation_.y += mouseDelta.x * sensitivity_.y;

		// X軸回転: 上下
		eulerRotation_.x += mouseDelta.y * sensitivity_.x;
		// 値を制限
		eulerRotation_.x = std::clamp(eulerRotation_.x,
			eulerRotateClampMinusX_, eulerRotateClampPlusX_);
	}

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

void FollowCamera::UpdateScreenShake() {

	// Playerが敵にダメージを与えた時
	if (isScreenShake_) {

		screenShakeTimer_ += GameTimer::GetScaledDeltaTime();

		// シェイクの残り時間を計算
		float remainingTime = screenShakeDuration_ - screenShakeTimer_;
		if (remainingTime > 0.0f) {

			float dampingFactor = remainingTime / screenShakeDuration_;
			float intensity = screenShakeIntensity_ * dampingFactor;

			float offsetX = RandomGenerator::Generate(-1.0f, 1.0f) * intensity;
			float offsetY = RandomGenerator::Generate(-1.0f, 1.0f) * intensity * 4.0f;
			float offsetZ = RandomGenerator::Generate(-1.0f, 1.0f) * intensity;

			Vector3 forward = Vector3(
				std::cos(transform_.rotation.y) * cos(transform_.rotation.x),
				std::sin(transform_.rotation.x),
				std::sin(transform_.rotation.y) * cos(transform_.rotation.x)
			);

			Vector3 right = Vector3(
				std::cos(transform_.rotation.y + std::numbers::pi_v<float> / 2.0f),
				0,
				std::sin(transform_.rotation.y + std::numbers::pi_v<float> / 2.0f));

			transform_.translation = transform_.translation + forward * offsetZ + right * offsetX;
			transform_.translation.y += offsetY;
		} else {

			// シェイク終了
			isScreenShake_ = false;
			screenShakeTimer_ = 0.0f;
		}
	}
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
	ImGui::DragFloat2("sensitivity", &sensitivity_.x, 0.001f);
	ImGui::DragFloat("eulerRotateClampPlusX", &eulerRotateClampPlusX_, 0.001f);
	ImGui::DragFloat("eulerRotateClampMinusX", &eulerRotateClampMinusX_, 0.001f);

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
	// yを初期化
	eulerRotation_.y = 0.0f;
	lerpRate_ = JsonAdapter::GetValue<float>(data, "lerpRate_");
	sensitivity_ = JsonAdapter::ToObject<Vector2>(data["sensitivity_"]);
	eulerRotateClampPlusX_ = JsonAdapter::GetValue<float>(data, "eulerRotateClampPlusX_");
	eulerRotateClampMinusX_ = JsonAdapter::GetValue<float>(data, "eulerRotateClampMinusX_");
}

void FollowCamera::SaveJson() {

	Json data;

	data["fovY_"] = fovY_;
	data["nearClip_"] = nearClip_;
	data["farClip_"] = farClip_;
	data["offsetTranslation_"] = JsonAdapter::FromObject<Vector3>(offsetTranslation_);
	data["eulerRotation_"] = JsonAdapter::FromObject<Vector3>(eulerRotation_);
	data["lerpRate_"] = lerpRate_;
	data["sensitivity_"] = JsonAdapter::FromObject<Vector2>(sensitivity_);
	data["eulerRotateClampPlusX_"] = eulerRotateClampPlusX_;
	data["eulerRotateClampMinusX_"] = eulerRotateClampMinusX_;

	JsonAdapter::Save("Camera/followCamera.json", data);
}