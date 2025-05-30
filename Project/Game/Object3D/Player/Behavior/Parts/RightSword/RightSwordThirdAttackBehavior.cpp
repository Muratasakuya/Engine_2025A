#include "RightSwordThirdAttackBehavior.h"

//============================================================================
//	include
//============================================================================
#include <Game/Object3D/Player/Parts/Base/BasePlayerParts.h>
#include <Engine/Particle/ParticleSystem.h>

//============================================================================
//  RightSwordThirdAttackBehavior classMethods
//============================================================================

RightSwordThirdAttackBehavior::RightSwordThirdAttackBehavior(
	const Json& data, const Vector3& initOffsetTranslate) {

	moveForward_ = std::make_unique<SimpleAnimation<Vector3>>();
	returnHand_ = std::make_unique<SimpleAnimation<Vector3>>();
	rotationAngleY_ = std::make_unique<SimpleAnimation<float>>();
	if (data.contains(attack3rdBehaviorJsonKey_) && data[attack3rdBehaviorJsonKey_].contains("MoveForward")) {

		moveForward_->FromJson(data[attack3rdBehaviorJsonKey_]["MoveForward"]);
		returnHand_->FromJson(data[attack3rdBehaviorJsonKey_]["ReturnHand"]);
		rotationAngleY_->FromJson(data[attack3rdBehaviorJsonKey_]["RotationAngleY"]);
		moveValue_ = JsonAdapter::GetValue<float>(data[attack3rdBehaviorJsonKey_], "moveValue_");
		basePosY_ = JsonAdapter::GetValue<float>(data[attack3rdBehaviorJsonKey_], "basePosY_");
		moveWaitTime_ = JsonAdapter::GetValue<float>(data[attack3rdBehaviorJsonKey_], "moveWaitTime_");
		rotateAngle_ = JsonAdapter::GetValue<float>(data[attack3rdBehaviorJsonKey_], "rotateAngle_");
		axis_ = JsonAdapter::ToObject<Vector3>(data[attack3rdBehaviorJsonKey_]["axis_"]);
		initRotationAngle_ = JsonAdapter::ToObject<Vector3>(data[attack3rdBehaviorJsonKey_]["initRotationAngle_"]);
		horizontalRotationAngle_ = JsonAdapter::ToObject<Vector3>(data[attack3rdBehaviorJsonKey_]["horizontalRotationAngle_"]);
	}

	// 戻ってくる座標を設定する
	returnHand_->move_.end = initOffsetTranslate;
}

void RightSwordThirdAttackBehavior::Execute(BasePlayerParts* parts) {

	// 前方に移動する
	UpdateMoveForward(parts);

	// 時間経過を待つ
	WaitMoveTime();

	// 元の場所に戻す
	UpdateReturnHand(parts);

	// 回転処理
	UpdateRotation(parts);

	// 体が回転している際の回転を設定する
	SetStartRotation(parts);
}

void RightSwordThirdAttackBehavior::UpdateMoveForward(BasePlayerParts* parts) {

	if (!moveForward_->IsStart()) {

		// 前方からlocal→world変換
		Quaternion orient = Quaternion::LookRotation(forwardDirection_, Vector3(0.0f, 1.0f, 0.0f));
		// local軸axis_をworld軸へ回す
		Vector3 worldAxis = Quaternion::RotateVector(axis_, orient);
		// その軸で既定の角度だけ回転
		Quaternion worldRotation =
			Quaternion::MakeRotateAxisAngleQuaternion(worldAxis, rotateAngle_);
		// 向きを決定
		Vector3 worldDirection =
			Quaternion::RotateVector(forwardDirection_, worldRotation);

		// 親のworldを取得
		Vector3 worldStart = parts->GetTransform().GetWorldPos();
		Vector3 worldEnd = worldStart + worldDirection * moveValue_;

		// worldの逆行列でlocalに戻す
		Matrix4x4 inverseParent = Matrix4x4::Inverse(parts->GetTransform().parent->matrix.world);
		Vector3 localEnd = Vector3::Transform(worldEnd, inverseParent);
		// Y座標を設定
		localEnd.y = basePosY_;

		moveForward_->move_.start = parts->GetTransform().translation;
		moveForward_->move_.end = localEnd;
		moveForward_->Start();
	}

	if (moveForward_->IsFinished()) {
		return;
	}

	// 値を補間
	Vector3 translation = parts->GetTransform().translation;
	moveForward_->LerpValue(translation);
	parts->SetTranslate(translation);
}

void RightSwordThirdAttackBehavior::UpdateRotation(BasePlayerParts* parts) {

	if (!rotationAngleY_->IsStart()) {

		// 初期回転を設定する
		Quaternion initRotation = IPlayerBehavior::CalRotationAxisAngle(initRotationAngle_);
		parts->SetRotate(initRotation);

		rotationAngleY_->Start();

		waitParticleStart_ = true;
	}

	if (rotationAngleY_->IsFinished()) {
		return;
	}

	if (waitParticleStart_ && !emitParticle_) {

		waitParticle_ += GameTimer::GetDeltaTime();
		if (waitParticle_ > 0.2f) {

			// effectを発生させる
			Vector3 effectPos = parts->GetTransform().GetWorldPos();
			effectPos += forwardDirection_ * 2.0f;
			effectPos.x += 0.6f;
			effectPos.y = 1.0f;
			particleSystem_->SetTranslate("electricEffectEmitter", effectPos);
			particleSystem_->Emit("electricEffectEmitter");
			emitParticle_ = true;
		}
	}

	// Y軸回転させる
	float rotationAngleY = 0.0f;
	rotationAngleY_->LerpValue(rotationAngleY);
	Quaternion rotation = parts->GetTransform().rotation;
	Quaternion newRotation = Quaternion::MakeRotateAxisAngleQuaternion(Vector3(1.0f, 0.0f, 0.0f), rotationAngleY);
	newRotation = Quaternion::Multiply(rotation, newRotation);
	parts->SetRotate(newRotation);
}

void RightSwordThirdAttackBehavior::UpdateReturnHand(BasePlayerParts* parts) {

	// 時間経過しきったら
	if (enableMoveFront_) {
		if (!returnHand_->IsStart()) {

			// 開始地点を設定
			returnHand_->move_.start = parts->GetTransform().translation;
			// animation開始
			returnHand_->Start();
		}
	}

	if (returnHand_->IsFinished()) {
		return;
	}

	// 値を補間
	Vector3 translation = parts->GetTransform().translation;
	returnHand_->LerpValue(translation);
	parts->SetTranslate(translation);
}

void RightSwordThirdAttackBehavior::SetStartRotation(BasePlayerParts* parts) {

	// 時間経過が終わったら
	if (rotationAngleY_->IsFinished()) {
		if (!setRotation_) {

			// 値を設定
			Quaternion rotation = IPlayerBehavior::CalRotationAxisAngle(horizontalRotationAngle_);
			rotation = Quaternion::Normalize(rotation);
			parts->SetRotate(rotation);

			// 設定完了
			setRotation_ = true;
		}
	} else {
		return;
	}
}

void RightSwordThirdAttackBehavior::WaitMoveTime() {

	// 最初のanimationが終わったら
	if (moveForward_->IsFinished() && !enableMoveFront_) {

		// 時間経過を進める
		moveWaitTimer_ += GameTimer::GetDeltaTime();
		if (moveWaitTimer_ > moveWaitTime_) {

			// 次のanimation開始
			enableMoveFront_ = true;
		}
	}
}

void RightSwordThirdAttackBehavior::Reset() {

	// 初期化する
	moveForward_->Reset();
	returnHand_->Reset();
	rotationAngleY_->Reset();
	moveWaitTimer_ = 0.0f;
	enableMoveFront_ = false;
	waitParticleStart_ = false;
	emitParticle_ = false;
	waitParticle_ = 0.0f;
	setRotation_ = false;
}

void RightSwordThirdAttackBehavior::ImGui() {

	ImGui::PushItemWidth(itemWidth_);

	ImGui::Text("forwardDirection: (%4.3f,%4.3f,%4.3f)",
		forwardDirection_.x, forwardDirection_.y, forwardDirection_.z);

	ImGui::DragFloat("moveWaitTime", &moveWaitTime_, 0.01f);
	ImGui::DragFloat("moveValue", &moveValue_, 0.01f);
	ImGui::DragFloat("basePosY", &basePosY_, 0.01f);
	ImGui::DragFloat("rotateAngle", &rotateAngle_, 0.01f);
	if (ImGui::DragFloat3("axis", &axis_.x, 0.01f)) {

		axis_ = axis_.Normalize();
	}
	ImGui::DragFloat3("initRotationAngle##RightSwordThirdAttackBehavior", &initRotationAngle_.x, 0.01f);
	ImGui::DragFloat3("horizontalRotationAngle##RightSwordThirdAttackBehavior", &horizontalRotationAngle_.x, 0.01f);

	if (ImGui::TreeNode("MoveForward")) {

		moveForward_->ImGui("RightSwordThirdAttackBehavior_moveForward_");
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("ReturnHand")) {

		returnHand_->ImGui("RightSwordThirdAttackBehavior_returnHand_");
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("RotationAngleY")) {

		rotationAngleY_->ImGui("RightSwordThirdAttackBehavior_rotationAngleY_");
		ImGui::TreePop();
	}

	ImGui::PopItemWidth();
}

void RightSwordThirdAttackBehavior::SaveJson(Json& data) {

	// 値の保存
	moveForward_->ToJson(data[attack3rdBehaviorJsonKey_]["MoveForward"]);
	returnHand_->ToJson(data[attack3rdBehaviorJsonKey_]["ReturnHand"]);
	rotationAngleY_->ToJson(data[attack3rdBehaviorJsonKey_]["RotationAngleY"]);
	data[attack3rdBehaviorJsonKey_]["moveValue_"] = moveValue_;
	data[attack3rdBehaviorJsonKey_]["basePosY_"] = basePosY_;
	data[attack3rdBehaviorJsonKey_]["moveWaitTime_"] = moveWaitTime_;
	data[attack3rdBehaviorJsonKey_]["rotateAngle_"] = rotateAngle_;
	data[attack3rdBehaviorJsonKey_]["axis_"] = JsonAdapter::FromObject<Vector3>(axis_);
	data[attack3rdBehaviorJsonKey_]["initRotationAngle_"] = JsonAdapter::FromObject<Vector3>(initRotationAngle_);
	data[attack3rdBehaviorJsonKey_]["horizontalRotationAngle_"] = JsonAdapter::FromObject<Vector3>(horizontalRotationAngle_);
}

void RightSwordThirdAttackBehavior::SetForwardDirection(const Vector3& direction) {

	forwardDirection_ = direction;
}