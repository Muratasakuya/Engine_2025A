#include "BodyFirstAttackBehavior.h"

//============================================================================
//	include
//============================================================================
#include <Game/Object3D/Player/Parts/Base/BasePlayerParts.h>
#include <Engine/Particle/ParticleSystem.h>

//============================================================================
//	BodyFirstAttackBehavior classMethods
//============================================================================

BodyFirstAttackBehavior::BodyFirstAttackBehavior(const Json& data) {

	moveStraight_ = std::make_unique<SimpleAnimation<Vector3>>();
	backLeftRotation_ = std::make_unique<SimpleAnimation<Vector3>>();
	forwardRightShift_ = std::make_unique<SimpleAnimation<Vector3>>();
	if (data.contains(attack1stBehaviorJsonKey_) && data[attack1stBehaviorJsonKey_].contains("MoveStraight")) {

		moveStraight_->FromJson(data[attack1stBehaviorJsonKey_]["MoveStraight"]);
		backLeftRotation_->FromJson(data[attack1stBehaviorJsonKey_]["BackLeftRotation"]);
		forwardRightShift_->FromJson(data[attack1stBehaviorJsonKey_]["ForwardRightShift"]);
		moveValue_ = JsonAdapter::GetValue<float>(data[attack1stBehaviorJsonKey_], "moveValue_");
	}

	// 相対角度はすべて0.0f
	backLeftRotation_->move_.start = Vector3::AnyInit(0.0f);
	forwardRightShift_->move_.start = Vector3::AnyInit(0.0f);
}

void BodyFirstAttackBehavior::Execute(BasePlayerParts* parts) {

	// 左後ろに回転
	UpdateBackLeftRotation(parts);
	// 直進移動
	UpdateMoveStraight(parts);
	// 左後ろに回転したあと正面に向かせる
	UpdateForwardRightShift(parts);
}

void BodyFirstAttackBehavior::UpdateMoveStraight(BasePlayerParts* parts) {

	if (!moveStraight_->IsStart()) {

		// 開始時の座標、回転を基にどれくらい進ませるか計算する
		// 初期座標を設定
		moveStraight_->move_.start = parts->GetTransform().translation;
		// 終了座標を設定
		Vector3 direction = parts->GetTransform().GetForward();
		direction.y = 0.0f;
		direction = direction.Normalize();
		moveStraight_->move_.end = parts->GetTransform().translation + (direction * moveValue_);

		moveStraight_->Start();
	}

	// 左後ろに回転が終わってから進ませる
	if (backLeftRotation_->IsFinished()) {

		// 値を補間
		Vector3 translation = parts->GetTransform().translation;
		moveStraight_->LerpValue(translation);
		parts->SetTranslate(translation);
	}
}

void BodyFirstAttackBehavior::UpdateBackLeftRotation(BasePlayerParts* parts) {

	if (!backLeftRotation_->IsStart()) {

		// animation開始
		backLeftRotation_->Start();
		if (!startRotation_.has_value()) {

			// 開始時の回転を記録
			startRotation_ = parts->GetTransform().rotation;

			startDirection_ = parts->GetTransform().GetForward();
		}
	}

	// 値を補間
	backLeftRotation_->LerpValue(rotationAngle_);
	// 回転を計算
	Quaternion deltaRotation = IPlayerBehavior::CalRotationAxisAngle(rotationAngle_);
	Quaternion rotation = Quaternion::Normalize(*startRotation_ * deltaRotation);
	parts->SetRotate(rotation);
}

void BodyFirstAttackBehavior::UpdateForwardRightShift(BasePlayerParts* parts) {

	// 左後ろに回転が終わったら
	if (backLeftRotation_->IsFinished()) {
		if (!forwardRightShift_->IsStart()) {

			// animation開始
			forwardRightShift_->Start();

			// particleを発生させる
			// effectを発生させる
			Vector3 effectPos = parts->GetTransform().translation;
			effectPos += startDirection_ * 6.0f;
			effectPos.y = 2.0f;
			particleSystem_->SetTranslate("hitEffectEmitter", effectPos);
			particleSystem_->Emit("hitEffectEmitter");
			emitParticle_ = true;
		}

		// 値を補間
		forwardRightShift_->LerpValue(rotationAngle_);
		// 回転を計算
		Quaternion deltaRotation = IPlayerBehavior::CalRotationAxisAngle(rotationAngle_);
		Quaternion rotation = Quaternion::Normalize(*startRotation_ * deltaRotation);
		parts->SetRotate(rotation);
	}
}

void BodyFirstAttackBehavior::Reset() {

	// 初期化する
	moveStraight_->Reset();
	backLeftRotation_->Reset();
	forwardRightShift_->Reset();
	rotationAngle_.Init();
	startRotation_ = std::nullopt;
}

void BodyFirstAttackBehavior::ImGui() {

	ImGui::PushItemWidth(itemWidth_);

	ImGui::DragFloat("moveValue##BodyFirstAttackBehavior", &moveValue_, 0.01f);

	ImGui::PopItemWidth();

	moveStraight_->ImGui("BodyFirstAttackBehavior_moveStraight_");
	backLeftRotation_->ImGui("BodyFirstAttackBehavior_backLeftRotation_");
	forwardRightShift_->ImGui("BodyFirstAttackBehavior_forwardRightShift_");
}

void BodyFirstAttackBehavior::SaveJson(Json& data) {

	// 値の保存
	moveStraight_->ToJson(data[attack1stBehaviorJsonKey_]["MoveStraight"]);
	backLeftRotation_->ToJson(data[attack1stBehaviorJsonKey_]["BackLeftRotation"]);
	forwardRightShift_->ToJson(data[attack1stBehaviorJsonKey_]["ForwardRightShift"]);
	data[attack1stBehaviorJsonKey_]["moveValue_"] = moveValue_;
}