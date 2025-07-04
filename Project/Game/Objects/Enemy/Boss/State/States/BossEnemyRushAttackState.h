#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/Enemy/Boss/State/Interface/BossEnemyIState.h>

// memo
// 計n回テレポートし、その都度設定された攻撃を行う
// 1回目: farにテレポートし3本の刃の攻撃
// 2回目: farにテレポートし3本の刃の攻撃
// 3回目: farにテレポートしチャージ攻撃

//============================================================================
//	BossEnemyRushAttackState class
//============================================================================
class BossEnemyRushAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyRushAttackState();
	~BossEnemyRushAttackState() = default;

	void Enter(BossEnemy& bossEnemy) override;

	void Update(BossEnemy& bossEnemy) override;

	void Exit(BossEnemy& bossEnemy) override;

	// imgui
	void ImGui(const BossEnemy& bossEnemy) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 現在の状態
	enum class State {

		Teleport, // テレポート
		Attack,   // 攻撃
		Cooldown  // 攻撃不可能状態
	};

	// 攻撃の種類
	struct AttackPattern {

		BossEnemyTeleportType teleportType;
		std::string animationName;
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 攻撃のパターン
	std::array<AttackPattern, 3> pattern_ = {
		AttackPattern{ BossEnemyTeleportType::Far, "bossEnemy_rushAttack" },
		AttackPattern{ BossEnemyTeleportType::Far, "bossEnemy_rushAttack" },
		AttackPattern{ BossEnemyTeleportType::Far, "bossEnemy_chargeAttack" }
	};

	// parameters
	float farRadius_;  // 扇形半径(遠くに移動)
	float nearRadius_; // 扇形半径(近くに移動)
	float halfAngle_;  // 扇形の半開き角

	// 座標
	Vector3 startPos_;  // 開始座標
	Vector3 targetPos_; // 目標座標

	float lerpTimer_;       // 座標補間の際の経過時間
	float lerpTime_;        // 座標補間にかける時間
	EasingType easingType_; // 補間の際のイージング

	float attackCoolTimer_; // 攻撃クールタイムの経過時間
	float attackCoolTime_;  // 攻撃クールタイム

	int maxAttackCount_;     // 攻撃回数
	int currentAttackCount_; // 現在の攻撃回数

	float fadeOutTime_;  // テレポート開始時の時間
	float fadeInTime_;   // テレポート終了時の時間
	float currentAlpha_; // α値

	float emitParticleOffsetY_; // particleの発生位置のオフセット

	//--------- functions ----------------------------------------------------

	// 各状態の更新
	void UpdateTeleport(BossEnemy& bossEnemy, float deltaTime);
	void UpdateAttack(BossEnemy& bossEnemy);
	void UpdateCooldown(BossEnemy& bossEnemy, float deltaTime);
};