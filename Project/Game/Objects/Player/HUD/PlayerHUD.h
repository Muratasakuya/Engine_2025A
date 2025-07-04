#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/Base/GameHPBar.h>
#include <Engine/Input/InputStructures.h>
#include <Game/Objects/Base/GameCommonStructures.h>
#include <Game/Objects/Player/Structures/PlayerStructures.h>

// c++
#include <utility>

//============================================================================
//	PlayerHUD class
//============================================================================
class PlayerHUD {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerHUD() = default;
	~PlayerHUD() = default;

	void Init();

	void Update();

	void ImGui();

	//--------- accessor -----------------------------------------------------

	void SetStatas(const PlayerStats& stats) { stats_ = stats; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 入力状態に応じて変化するsprite
	struct InputStateSprite {

		std::unique_ptr<GameEntity2D> staticSprite;
		std::unordered_map<InputType, std::unique_ptr<GameEntity2D>> dynamicSprites;

		uint32_t index; // spriteを左から並べた時の順番

		// groupの名前
		const std::string groupName = "PlayerHUD";

		void Init(uint32_t spriteIndex, const std::string& staticSpriteTextureName,
			const std::unordered_map<InputType, std::string>& dynamicSpritesTextureName);

		void ChangeDynamicSprite(InputType type);

		void SetTranslation(const Vector2& leftSpriteTranslation,
			float dynamicSpriteOffsetY, float operateSpriteSpancingX);

		void SetSize(const Vector2& staticSpriteSize,
			const Vector2& dynamicSpriteSize_);
	};

	//--------- variables ----------------------------------------------------

	// ステータス
	PlayerStats stats_;
	// 現在の入力状態
	InputType inputType_;

	//----------- stats ------------------------//

	// HP背景
	std::unique_ptr<GameEntity2D> hpBackground_;
	GameCommon::HUDInitParameter hpBackgroundParameter_;
	// HP残量
	std::unique_ptr<GameHPBar> hpBar_;
	GameCommon::HUDInitParameter hpBarParameter_;
	// スキル値
	std::unique_ptr<GameHPBar> skilBar_;
	GameCommon::HUDInitParameter skilBarParameter_;

	// 名前文字表示
	std::unique_ptr<GameEntity2D> nameText_;
	GameCommon::HUDInitParameter nameTextParameter_;

	//----------- operate ----------------------//

	// 操作方法表示
	InputStateSprite attack_;  // 攻撃
	InputStateSprite dash_;    // ダッシュ
	InputStateSprite skil_;    // スキル
	InputStateSprite special_; // 必殺

	// parameters
	Vector2 leftSpriteTranslation_; // 左端のsprite座標
	float dynamicSpriteOffsetY_;    // 入力状態に応じて変化するspriteのオフセットY座標
	float operateSpriteSpancingX_;  // indexに応じてオフセットをかける
	// それぞれのspriteのサイズ
	Vector2 staticSpriteSize_;
	Vector2 dynamicSpriteSize_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// init
	void InitSprite();

	// update
	void UpdateSprite();

	// helper
	void ChangeAllOperateSprite();
	void SetAllOperateTranslation();
	void SetAllOperateSize();
};