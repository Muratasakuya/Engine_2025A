#include "LeftSwordThirdAttackBehavior.h"

//============================================================================
//	include
//============================================================================
#include <Game/Object3D/Player/Parts/Base/BasePlayerParts.h>

//============================================================================
//  LeftSwordThirdAttackBehavior classMethods
//============================================================================

LeftSwordThirdAttackBehavior::LeftSwordThirdAttackBehavior(
	const Json& data, const Vector3& initOffsetTranslate) {

	moveForward_ = std::make_unique<SimpleAnimation<Vector3>>();
	returnHand_ = std::make_unique<SimpleAnimation<Vector3>>();
	if (data.contains(attack3rdBehaviorJsonKey_) && data[attack3rdBehaviorJsonKey_].contains("MoveForward")) {

		moveForward_->FromJson(data[attack3rdBehaviorJsonKey_]["MoveForward"]);
		returnHand_->FromJson(data[attack3rdBehaviorJsonKey_]["ReturnHand"]);
		moveValue_ = JsonAdapter::GetValue<float>(data[attack3rdBehaviorJsonKey_], "moveValue_");
		moveWaitTime_ = JsonAdapter::GetValue<float>(data[attack3rdBehaviorJsonKey_], "moveWaitTime_");
		rotateAngle_ = JsonAdapter::GetValue<float>(data[attack3rdBehaviorJsonKey_], "rotateAngle_");
		axis_ = JsonAdapter::ToObject<Vector3>(data[attack3rdBehaviorJsonKey_]["axis_"]);
	}

	// 戻ってくる座標を設定する
	returnHand_->move_.end = initOffsetTranslate;
}

void LeftSwordThirdAttackBehavior::Execute(BasePlayerParts* parts) {

	// 前方に移動する
	UpdateMoveForward(parts);

	// 時間経過を待つ
	WaitMoveTime();

	// 元の場所に戻す
	UpdateReturnHand(parts);
}

void LeftSwordThirdAttackBehavior::UpdateMoveForward(BasePlayerParts* parts) {

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

void LeftSwordThirdAttackBehavior::UpdateReturnHand(BasePlayerParts* parts) {

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

void LeftSwordThirdAttackBehavior::WaitMoveTime() {

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

void LeftSwordThirdAttackBehavior::Reset() {

	// 初期化する
	moveForward_->Reset();
	returnHand_->Reset();
	moveWaitTimer_ = 0.0f;
	enableMoveFront_ = false;
}

void LeftSwordThirdAttackBehavior::ImGui() {

	ImGui::PushItemWidth(itemWidth_);

	ImGui::Text("forwardDirection: (%4.3f,%4.3f,%4.3f)",
		forwardDirection_.x, forwardDirection_.y, forwardDirection_.z);

	ImGui::DragFloat("moveWaitTime", &moveWaitTime_, 0.01f);
	ImGui::DragFloat("moveValue", &moveValue_, 0.01f);
	ImGui::DragFloat("rotateAngle", &rotateAngle_, 0.01f);
	if (ImGui::DragFloat3("axis", &axis_.x, 0.01f)) {
	
		axis_ = axis_.Normalize();
	}

	if (ImGui::TreeNode("MoveForward")) {

		moveForward_->ImGui("LeftSwordThirdAttackBehavior_moveForward_");
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("ReturnHand")) {

		returnHand_->ImGui("LeftSwordThirdAttackBehavior_returnHand_");
		ImGui::TreePop();
	}

	ImGui::PopItemWidth();
}

void LeftSwordThirdAttackBehavior::SaveJson(Json& data) {

	// 値の保存
	moveForward_->ToJson(data[attack3rdBehaviorJsonKey_]["MoveForward"]);
	returnHand_->ToJson(data[attack3rdBehaviorJsonKey_]["ReturnHand"]);
	data[attack3rdBehaviorJsonKey_]["moveValue_"] = moveValue_;
	data[attack3rdBehaviorJsonKey_]["moveWaitTime_"] = moveWaitTime_;
	data[attack3rdBehaviorJsonKey_]["rotateAngle_"] = rotateAngle_;
	data[attack3rdBehaviorJsonKey_]["axis_"] = JsonAdapter::FromObject<Vector3>(axis_);
}

void LeftSwordThirdAttackBehavior::SetForwardDirection(const Vector3& direction) {

	forwardDirection_ = direction;
}