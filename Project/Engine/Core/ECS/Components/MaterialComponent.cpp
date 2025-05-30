#include "MaterialComponent.h"

//============================================================================
//	include
//============================================================================
#include <Game/Time/GameTimer.h>
#include <Lib/Adapter/JsonAdapter.h>

// imgui
#include <imgui.h>

//============================================================================
//	MaterialComponent classMethods
//============================================================================

void MaterialComponent::Init() {

	material.Init();
	uvTransform.scale = Vector3::AnyInit(1.0f);
	prevUVTransform.scale = Vector3::AnyInit(1.0f);
}

void MaterialComponent::UpdateUVTransform() {

	// 値に変更がなければ更新しない
	if (uvTransform == prevUVTransform) {
		return;
	}

	// uvの更新
	material.uvTransform = Matrix4x4::MakeAffineMatrix(
		uvTransform.scale, uvTransform.rotate, uvTransform.translate);

	// 値を保存
	prevUVTransform = uvTransform;
}

void MaterialComponent::ImGui(float itemSize) {

	// 色
	ImGui::SeparatorText("Color");

	ImGui::PushItemWidth(itemSize);
	ImGui::ColorEdit4("color", &material.color.r);
	ImGui::Text("R:%4.3f G:%4.3f B:%4.3f A:%4.3f",
		material.color.r, material.color.g,
		material.color.b, material.color.a);

	// 発行色
	ImGui::ColorEdit3("emissionColor", &material.emissionColor.x);
	ImGui::Text("R:%4.3f G:%4.3f B:%4.3f",
		material.emissionColor.x, material.emissionColor.y,
		material.emissionColor.z);
	ImGui::DragFloat("emissiveIntensity", &material.emissiveIntensity, 0.01f);

	// UV
	ImGui::SeparatorText("UV");

	// transform
	ImGui::DragFloat2("uvTranslate", &uvTransform.translate.x, 0.1f);
	ImGui::SliderAngle("uvRotate", &uvTransform.rotate.z);
	ImGui::DragFloat2("uvScale", &uvTransform.scale.x, 0.1f);

	// Lighting
	ImGui::SeparatorText("Lighting");

	ImGui::SliderInt("enableLighting", &material.enableLighting, 0, 1);
	ImGui::SliderInt("blinnPhongReflection", &material.enableBlinnPhongReflection, 0, 1);

	if (material.enableBlinnPhongReflection) {

		ImGui::ColorEdit3("specularColor", &material.specularColor.x);
		ImGui::DragFloat("phongRefShininess", &material.phongRefShininess, 0.01f);
	}

	ImGui::SliderInt("enableImageBasedLighting", &material.enableImageBasedLighting, 0, 1);
	if (material.enableImageBasedLighting) {

		ImGui::DragFloat("environmentCoefficient", &material.environmentCoefficient, 0.001f, 0.0f, 4.0f);
	}

	ImGui::DragFloat("shadowRate", &material.shadowRate, 0.01f, 0.0f, 8.0f);
	ImGui::PopItemWidth();
}

void MaterialComponent::ToJson(Json& data) {

	// Material
	// color
	data["color"] = material.color.ToJson();
	data["emissionColor"] = material.emissionColor.ToJson();
	data["emissiveIntensity"] = material.emissiveIntensity;
	// lighting
	data["enableLighting"] = material.enableLighting;
	data["enableHalfLambert"] = material.enableHalfLambert;
	data["enableBlinnPhongReflection"] = material.enableBlinnPhongReflection;
	data["enableImageBasedLighting"] = material.enableImageBasedLighting;
	data["phongRefShininess"] = material.phongRefShininess;
	data["specularColor"] = material.specularColor.ToJson();
	data["shadowRate"] = material.shadowRate;
	data["environmentCoefficient"] = material.environmentCoefficient;

	// UV
	data["uvScale"] = uvTransform.scale.ToJson();
	data["uvRotate"] = uvTransform.rotate.ToJson();
	data["uvTranslate"] = uvTransform.translate.ToJson();
}

void MaterialComponent::FromJson(const Json& data) {

	// Material
	// color
	material.color = JsonAdapter::ToObject<Color>(data["color"]);
	material.emissionColor = JsonAdapter::ToObject<Vector3>(data["emissionColor"]);
	material.emissiveIntensity = data["emissiveIntensity"];
	// lighting
	material.enableLighting = data["enableLighting"];
	material.enableHalfLambert = data["enableHalfLambert"];
	material.enableBlinnPhongReflection = data["enableBlinnPhongReflection"];
	material.enableBlinnPhongReflection = data["enableBlinnPhongReflection"];
	//material.enableImageBasedLighting = JsonAdapter::GetValue<bool>(data, "enableImageBasedLighting");
	material.phongRefShininess = data["phongRefShininess"];
	material.specularColor = JsonAdapter::ToObject<Vector3>(data["specularColor"]);
	material.shadowRate = data["shadowRate"];
	//material.environmentCoefficient = JsonAdapter::GetValue<float>(data, "environmentCoefficient");

	// UV
	uvTransform.scale = JsonAdapter::ToObject<Vector3>(data["uvScale"]);
	uvTransform.rotate = JsonAdapter::ToObject<Vector3>(data["uvRotate"]);
	uvTransform.translate = JsonAdapter::ToObject<Vector3>(data["uvTranslate"]);
}