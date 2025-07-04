#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Entity/Interface/IGameEntity.h>

// components
#include <Engine/Core/ECS/Components/SpriteComponent.h>
// front
class BaseCamera;

//============================================================================
//	GameEntity2D class
//============================================================================
class GameEntity2D :
	public IGameEntity {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameEntity2D() = default;
	virtual ~GameEntity2D() = default;

	void Init(const std::string& textureName, const std::string& name, const std::string& groupName);

	void DerivedInit() override {};

	// imgui
	void ImGui() override {};
	virtual void DerivedImGui() override {}

	//--------- accessor -----------------------------------------------------

	/*---------- setter ----------*/

	// transform
	// 座標
	void SetTranslation(const Vector2& translation) { transform_->translation = translation; }
	void SetCenterTranslation();
	void ProjectToScreen(const Vector3& translation, const BaseCamera& camera);
	// サイズ
	void SetSize(const Vector2& size) { transform_->size = size; }
	void SetTextureSize(const Vector2& size) { transform_->textureSize = size; }
	void SetTextureLeftTop(const Vector2& leftTop) { transform_->textureLeftTop = leftTop; }
	// アンカー
	void SetAnchor(const Vector2& anchor) { transform_->anchorPoint = anchor; }
	// 親
	void SetParent(const Transform2DComponent& parent) { transform_->parent = &parent; }
	// 頂点オフセット
	void SetVertexOffset(uint32_t index, const Vector2& offset) { transform_->vertexOffset_[index] = offset; };

	// material
	// 色
	void SetColor(const Color& color) { material_->material.color = color; }
	void SetAlpha(float alpha) { material_->material.color.a = alpha; }
	void SetEmissiveIntensity(float intensity) { material_->material.emissiveIntensity = intensity; }
	void SetEmissionColor(const Vector3& color) { material_->material.emissionColor = color; }
	// UV
	void SetUVTranslationX(float uvX) { material_->uvTransform.translation.x = uvX; }
	void SetUVTranslationY(float uvY) { material_->uvTransform.translation.y = uvY; }
	void SetUVScaleX(float uvX) { material_->uvTransform.scale.x = uvX; }
	void SetUVScaleY(float uvY) { material_->uvTransform.scale.y = uvY; }

	// sprite
	void SetTextureName(const std::string& textureName) { sprite_->SetTextureName(textureName); }
	void SetAlphaTextureName(const std::string& textureName) { sprite_->SetAlphaTextureName(textureName); }
	void SetSpriteLayer(SpriteLayer layer) { sprite_->SetLayer(layer); }
	void SetPostProcessEnable(bool enable) { sprite_->SetPostProcessEnable(enable); }

	/*---------- getter ----------*/

	// transform
	const Transform2DComponent& GetTransform() const { return *transform_; }
	const Vector2& GetTranslation() const { return transform_->translation; }
	const Vector2& GetSize() const { return transform_->size; }
	const Vector2& GetTextureSize() const { return transform_->textureSize; }

	// material
	const Color& GetColor() const { return material_->material.color; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// components
	// transform
	Transform2DComponent* transform_;
	// material
	SpriteMaterialComponent* material_;
	// sprite
	SpriteComponent* sprite_;
};