#include "BodySecondAttackBehavior.h"

//============================================================================
//	include
//============================================================================
#include <Game/Object3D/Player/Parts/Base/BasePlayerParts.h>
#include <Engine/Particle/ParticleSystem.h>

//============================================================================
//	BodySecondAttackBehavior classMethods
//============================================================================

BodySecondAttackBehavior::BodySecondAttackBehavior(const Json& data) {

	moveFrontLeft_.moveAnimation = std::make_unique<SimpleAnimation<Vector3>>();
	moveFrontRight_.moveAnimation = std::make_unique<SimpleAnimation<Vector3>>();
	moveCenter_.moveAnimation = std::make_unique<SimpleAnimation<Vector3>>();

	frontLeftRotation_ = std::make_unique<SimpleAnimation<Vector3>>();
	frontRightRotation_ = std::make_unique<SimpleAnimation<Vector3>>();
	returnCenterRotation_ = std::make_unique<SimpleAnimation<Vector3>>();
	if (data.contains(attack2ndBehaviorJsonKey_) && data[attack2ndBehaviorJsonKey_].contains("MoveFrontLeft")) {

		moveFrontLeft_.moveValue = JsonAdapter::GetValue<float>(data[attack2ndBehaviorJsonKey_], "moveFrontLeft_moveValue");
		moveFrontLeft_.direction = JsonAdapter::ToObject<Vector3>(data[attack2ndBehaviorJsonKey_]["moveFrontLeft_direction"]);
		moveFrontLeft_.moveAnimation->FromJson(data[attack2ndBehaviorJsonKey_]["MoveFrontLeft"]);
		frontLeftRotation_->FromJson(data[attack2ndBehaviorJsonKey_]["FrontLeftRotation"]);

		moveFrontRight_.moveValue = JsonAdapter::GetValue<float>(data[attack2ndBehaviorJsonKey_], "moveFrontRight_moveValue");
		moveFrontRight_.direction = JsonAdapter::ToObject<Vector3>(data[attack2ndBehaviorJsonKey_]["moveFrontRight_direction"]);
		moveFrontRight_.moveAnimation->FromJson(data[attack2ndBehaviorJsonKey_]["MoveFrontRight"]);
		frontRightRotation_->FromJson(data[attack2ndBehaviorJsonKey_]["FrontRightRotation"]);

		moveCenter_.moveValue = JsonAdapter::GetValue<float>(data[attack2ndBehaviorJsonKey_], "moveCenter_moveValue");
		moveCenter_.direction = JsonAdapter::ToObject<Vector3>(data[attack2ndBehaviorJsonKey_]["moveCenter_direction"]);
		moveCenter_.moveAnimation->FromJson(data[attack2ndBehaviorJsonKey_]["MoveCenter"]);
		returnCenterRotation_->FromJson(data[attack2ndBehaviorJsonKey_]["ReturnCenterRotation"]);
	}
}

void BodySecondAttackBehavior::Execute(BasePlayerParts* parts) {

	// 左前に行く
	UpdateFrontLeftStep(parts);
	UpdateFrontLeftRotation(parts);

	// 右前に大きく動く
	UpdateFrontRightStep(parts);
	UpdateFrontRightRotation(parts);

	// 最後、中心に戻るように左前に行く
	UpdateReturnCenterStep(parts);
	UpdateReturnCenterRotation(parts);

	particleSystem_->FrequencyEmit("attackFieldEmitter");
}

void BodySecondAttackBehavior::UpdateFrontLeftStep(BasePlayerParts* parts) {

	if (!moveFrontLeft_.moveAnimation->IsStart()) {

		forwardDirection_ = parts->GetTransform().GetForward();
		forwardDirection_.y = 0.0f;
		forwardDirection_ = Vector3::Normalize(forwardDirection_);

		// 向き、座標を計算してanimation開始
		Start(parts, moveFrontLeft_);

		// 状態を設定
		currentState_ = State::FrontLeftStep;

		startDirection_ = parts->GetTransform().GetForward();

		// particleを発生させる
		Vector3 effectPos = parts->GetTransform().translation;
		effectPos += startDirection_ * 6.0f;
		effectPos.y = 2.0f;
		particleSystem_->SetTranslate("hitEffectEmitter", effectPos);
		particleSystem_->Emit("hitEffectEmitter");
		emitParticle_ = true;
	}

	if (moveFrontLeft_.moveAnimation->IsFinished()) {
		return;
	}

	// 値を補間
	Vector3 translation = parts->GetTransform().translation;
	moveFrontLeft_.moveAnimation->LerpValue(translation);
	parts->SetTranslate(translation);
}

void BodySecondAttackBehavior::UpdateFrontRightStep(BasePlayerParts* parts) {

	// 1個前のanimationが終了したら開始する
	if (!moveFrontRight_.moveAnimation->IsStart()) {
		if (moveFrontLeft_.moveAnimation->IsFinished()) {

			// 向き、座標を計算してanimation開始
			Start(parts, moveFrontRight_);

			// 状態を設定
			currentState_ = State::FrontRightStep;

			// particleを発生させる
			Vector3 effectPos = parts->GetTransform().translation;
			effectPos += startDirection_ * 6.0f;
			effectPos.y = 2.0f;
			particleSystem_->SetTranslate("hitEffectEmitter", effectPos);
			particleSystem_->Emit("hitEffectEmitter");
			emitParticle_ = true;
		}
	}

	if (moveFrontRight_.moveAnimation->IsFinished()) {
		return;
	}

	// 値を補間
	Vector3 translation = parts->GetTransform().translation;
	moveFrontRight_.moveAnimation->LerpValue(translation);
	parts->SetTranslate(translation);
}

void BodySecondAttackBehavior::UpdateReturnCenterStep(BasePlayerParts* parts) {

	// 1個前のanimationが終了したら開始する
	if (!moveCenter_.moveAnimation->IsStart()) {
		if (moveFrontRight_.moveAnimation->IsFinished()) {

			// 向き、座標を計算してanimation開始
			Start(parts, moveCenter_);

			// 状態を設定
			currentState_ = State::ReturnCenterStep;

			// particleを発生させる
			Vector3 effectPos = parts->GetTransform().translation;
			effectPos += startDirection_ * 6.0f;
			effectPos.y = 2.0f;
			particleSystem_->SetTranslate("hitEffectEmitter", effectPos);
			particleSystem_->Emit("hitEffectEmitter");
			emitParticle_ = true;
		}
	}

	if (moveCenter_.moveAnimation->IsFinished()) {
		return;
	}

	// 値を補間
	Vector3 translation = parts->GetTransform().translation;
	moveCenter_.moveAnimation->LerpValue(translation);
	parts->SetTranslate(translation);
}

void BodySecondAttackBehavior::UpdateFrontLeftRotation(BasePlayerParts* parts) {

	if (!frontLeftRotation_->IsStart()) {

		// animation開始
		frontLeftRotation_->Start();
		// 開始時の回転を記録
		startRotation_ = parts->GetTransform().rotation;
	}

	if (frontLeftRotation_->IsFinished()) {
		return;
	}

	// 値を補間
	frontLeftRotation_->LerpValue(rotationAngle_);
	// 回転を計算
	Quaternion deltaRotation = IPlayerBehavior::CalRotationAxisAngle(rotationAngle_);
	Quaternion rotation = Quaternion::Normalize(startRotation_ * deltaRotation);
	parts->SetRotate(rotation);
}

void BodySecondAttackBehavior::UpdateFrontRightRotation(BasePlayerParts* parts) {

	// 右前に大きく動き終わったら
	if (frontLeftRotation_->IsFinished()) {
		if (!frontRightRotation_->IsStart()) {

			// animation開始
			frontRightRotation_->Start();
			// 初期値を設定
			frontRightRotation_->move_.start = rotationAngle_;
		}
	}

	if (frontRightRotation_->IsFinished()) {
		return;
	}

	// 値を補間
	frontRightRotation_->LerpValue(rotationAngle_);
	// 回転を計算
	Quaternion deltaRotation = IPlayerBehavior::CalRotationAxisAngle(rotationAngle_);
	Quaternion rotation = Quaternion::Normalize(startRotation_ * deltaRotation);
	parts->SetRotate(rotation);
}

void BodySecondAttackBehavior::UpdateReturnCenterRotation(BasePlayerParts* parts) {

	// 左前に行き終わったら
	if (frontRightRotation_->IsFinished()) {
		if (!returnCenterRotation_->IsStart()) {

			// animation開始
			returnCenterRotation_->Start();
			// 初期値を設定
			returnCenterRotation_->move_.start = rotationAngle_;
		}
	}

	if (returnCenterRotation_->IsFinished()) {
		return;
	}

	// 値を補間
	returnCenterRotation_->LerpValue(rotationAngle_);
	// 回転を計算
	Quaternion deltaRotation = IPlayerBehavior::CalRotationAxisAngle(rotationAngle_);
	Quaternion rotation = Quaternion::Normalize(startRotation_ * deltaRotation);
	parts->SetRotate(rotation);
}

void BodySecondAttackBehavior::Start(BasePlayerParts* parts, MoveStructure& moveStructure) {

	// 開始座標設定
	moveStructure.moveAnimation->move_.start = parts->GetTransform().translation;

	// 正規化
	moveStructure.direction = moveStructure.direction.Normalize();

	// 向きを計算
	Vector3 right = Vector3::Normalize(Vector3::Cross(Vector3(0.0f, 1.0f, 0.0f), forwardDirection_));
	Vector3 direction = (right * moveStructure.direction.x) + (forwardDirection_ * moveStructure.direction.z);
	direction.y = 0.0f;
	direction = Vector3::Normalize(direction);

	// 終了位置を設定
	moveStructure.moveAnimation->move_.end =
		parts->GetTransform().translation + (direction * moveStructure.moveValue);

	// animation開始
	moveStructure.moveAnimation->Start();
}

const char* BodySecondAttackBehavior::ToStringState(State state) {

	switch (state) {
	case State::FrontLeftStep:   return "FrontLeftStep";
	case State::FrontRightStep:  return "FrontRightStep";
	case State::ReturnCenterStep:return "ReturnCenterStep";
	default:                     return "Unknown";
	}
}

void BodySecondAttackBehavior::Reset() {

	// 初期化する
	moveFrontLeft_.moveAnimation->Reset();
	moveFrontRight_.moveAnimation->Reset();
	moveCenter_.moveAnimation->Reset();
	frontLeftRotation_->Reset();
	frontRightRotation_->Reset();
	returnCenterRotation_->Reset();
}

void BodySecondAttackBehavior::ImGui() {

	ImGui::PushItemWidth(itemWidth_);

	ImGui::Text(std::format("currentState: {}", ToStringState(currentState_)).c_str());

	if (ImGui::TreeNode("MoveFrontLeft")) {

		ImGui::DragFloat("moveValue##moveFrontLeft_", &moveFrontLeft_.moveValue, 0.01f);
		ImGui::DragFloat3("direction##moveFrontLeft_", &moveFrontLeft_.direction.x, 0.001f);

		ImGui::SeparatorText("Move");

		moveFrontLeft_.moveAnimation->ImGui("BodySecondAttackBehavior_moveFrontLeft_");

		ImGui::SeparatorText("Rotation");

		frontLeftRotation_->ImGui("BodySecondAttackBehavior_frontLeftRotation_");
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("MoveFrontRight")) {

		ImGui::DragFloat("moveValue##moveFrontRight_", &moveFrontRight_.moveValue, 0.01f);
		ImGui::DragFloat3("direction##moveFrontRight_", &moveFrontRight_.direction.x, 0.001f);

		ImGui::SeparatorText("Move");

		moveFrontRight_.moveAnimation->ImGui("BodySecondAttackBehavior_moveFrontRight_");

		ImGui::SeparatorText("Rotation");

		frontRightRotation_->ImGui("BodySecondAttackBehavior_frontRightRotation_");
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("MoveCenter")) {

		ImGui::DragFloat("moveValue##moveCenter_", &moveCenter_.moveValue, 0.01f);
		ImGui::DragFloat3("direction##moveCenter_", &moveCenter_.direction.x, 0.001f);

		ImGui::SeparatorText("Move");

		moveCenter_.moveAnimation->ImGui("BodySecondAttackBehavior_moveCenter_");

		ImGui::SeparatorText("Rotation");

		returnCenterRotation_->ImGui("BodySecondAttackBehavior_returnCenterRotation_");
		ImGui::TreePop();
	}

	ImGui::PopItemWidth();
}

void BodySecondAttackBehavior::SaveJson(Json& data) {

	// 値の保存
	data[attack2ndBehaviorJsonKey_]["moveFrontLeft_moveValue"] = moveFrontLeft_.moveValue;
	data[attack2ndBehaviorJsonKey_]["moveFrontLeft_direction"] = JsonAdapter::FromObject<Vector3>(moveFrontLeft_.direction);
	moveFrontLeft_.moveAnimation->ToJson(data[attack2ndBehaviorJsonKey_]["MoveFrontLeft"]);
	frontLeftRotation_->ToJson(data[attack2ndBehaviorJsonKey_]["FrontLeftRotation"]);

	data[attack2ndBehaviorJsonKey_]["moveFrontRight_moveValue"] = moveFrontRight_.moveValue;
	data[attack2ndBehaviorJsonKey_]["moveFrontRight_direction"] = JsonAdapter::FromObject<Vector3>(moveFrontRight_.direction);
	moveFrontRight_.moveAnimation->ToJson(data[attack2ndBehaviorJsonKey_]["MoveFrontRight"]);
	frontRightRotation_->ToJson(data[attack2ndBehaviorJsonKey_]["FrontRightRotation"]);

	data[attack2ndBehaviorJsonKey_]["moveCenter_moveValue"] = moveCenter_.moveValue;
	data[attack2ndBehaviorJsonKey_]["moveCenter_direction"] = JsonAdapter::FromObject<Vector3>(moveCenter_.direction);
	moveCenter_.moveAnimation->ToJson(data[attack2ndBehaviorJsonKey_]["MoveCenter"]);
	returnCenterRotation_->ToJson(data[attack2ndBehaviorJsonKey_]["ReturnCenterRotation"]);
}