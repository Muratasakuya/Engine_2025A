#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Entity/GameEntity3D.h>

// weapon
#include <Game/Objects/Enemy/Boss/Entity/BossEnemyWeapon.h>
// state
#include <Game/Objects/Enemy/Boss/State/BossEnemyStateController.h>
// HUD
#include <Game/Objects/Enemy/Boss/HUD/BossEnemyHUD.h>

// front
class Player;

//============================================================================
//	BossEnemy class
//============================================================================
class BossEnemy :
	public GameEntity3D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemy() = default;
	~BossEnemy() = default;

	void DerivedInit() override;

	void Update() override;

	void DerivedImGui() override;

	/*-------- collision ----------*/

	// 衝突コールバック関数
	void OnCollisionEnter([[maybe_unused]] const CollisionBody* collisionBody) override;

	//--------- accessor -----------------------------------------------------

	void SetPlayer(const Player* player);
	void SetFollowCamera(const FollowCamera* followCamera);

	void SetAlpha(float alpha);
	void SetCastShadow(bool cast);
	void SetDecreaseToughnessProgress(float progress);

	Vector3 GetWeaponTranslation() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const Player* player_;

	// 使用する武器
	std::unique_ptr<BossEnemyWeapon> weapon_;

	// 状態の管理
	std::unique_ptr<BossEnemyStateController> stateController_;

	// HUD
	std::unique_ptr<BossEnemyHUD> hudSprites_;

	// parameters
	Transform3DComponent initTransform_; // 初期化時の値
	BossEnemyStats stats_;               // ステータス

	// editor
	int selectedPhaseIndex_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// init
	void InitWeapon();
	void InitAnimations();
	void InitCollision();
	void InitState();
	void InitHUD();

	// helper
	void SetInitTransform();
};