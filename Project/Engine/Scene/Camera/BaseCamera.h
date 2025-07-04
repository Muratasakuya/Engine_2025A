#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/ECS/Components/TransformComponent.h>

// imgui
#include <imgui.h>

//============================================================================
//	BaseCamera class
//============================================================================
class BaseCamera {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BaseCamera();
	virtual ~BaseCamera() = default;

	virtual void Init() = 0;

	virtual void Update() = 0;

	virtual void ImGui() = 0;

	//--------- accessor -----------------------------------------------------

	void SetParent(const Transform3DComponent* parent) { transform_.parent = parent; };
	void SetTranslation(const Vector3& translation) { transform_.translation = translation; }
	void SetEulerRotation(const Vector3& eulerRotation) { transform_.eulerRotate = eulerRotation; }

	float GetFovY() const { return fovY_; }
	float GetNearClip() const { return nearClip_; }
	float GetFarClip() const { return farClip_; }

	const Transform3DComponent& GetTransform() const { return transform_; }

	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	const Matrix4x4& GetBillboardMatrix() const { return billboardMatrix_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const float itemWidth_ = 224.0f;

	float fovY_;
	float nearClip_;
	float farClip_;
	float aspectRatio_;

	Transform3DComponent transform_;
	Vector3 eulerRotation_;

	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;

	Matrix4x4 viewProjectionMatrix_;

	Matrix4x4 billboardMatrix_;

	//--------- functions ----------------------------------------------------

	void CalBillboardMatrix();
};