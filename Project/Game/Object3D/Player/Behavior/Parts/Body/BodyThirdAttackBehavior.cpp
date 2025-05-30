#include "BodyThirdAttackBehavior.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Renderer/LineRenderer.h>
#include <Engine/Particle/ParticleSystem.h>
#include <Game/Camera/FollowCamera.h>
#include <Game/Object3D/Player/Parts/Base/BasePlayerParts.h>

//============================================================================
//	BodyThirdAttackBehavior classMethods
//============================================================================

BodyThirdAttackBehavior::BodyThirdAttackBehavior(
	const Json& data, FollowCamera* followCamera) {

	followCamera_ = nullptr;
	followCamera_ = followCamera;

	moveBack_ = std::make_unique<SimpleAnimation<Vector3>>();
	moveFront_ = std::make_unique<SimpleAnimation<Vector3>>();

	backRotation_ = std::make_unique<SimpleAnimation<Vector3>>();
	frontRotation_ = std::make_unique<SimpleAnimation<Vector3>>();

	moveKeyframeAnimation_ = std::make_unique<SimpleAnimation<Vector3>>();
	rotationAngleZ_ = std::make_unique<SimpleAnimation<float>>();
	if (data.contains(attack3rdBehaviorJsonKey_) && data[attack3rdBehaviorJsonKey_].contains("MoveBack")) {

		moveBack_->FromJson(data[attack3rdBehaviorJsonKey_]["MoveBack"]);
		moveFront_->FromJson(data[attack3rdBehaviorJsonKey_]["MoveFront"]);
		backRotation_->FromJson(data[attack3rdBehaviorJsonKey_]["BackRotation"]);
		frontRotation_->FromJson(data[attack3rdBehaviorJsonKey_]["FrontRotation"]);

		moveKeyframeAnimation_->FromJson(data[attack3rdBehaviorJsonKey_]["MoveKeyframeAnimation"]);
		rotationAngleZ_->FromJson(data[attack3rdBehaviorJsonKey_]["RotationAngleZ"]);

		moveWaitTime_ = JsonAdapter::GetValue<float>(data[attack3rdBehaviorJsonKey_], "moveWaitTime_");
		moveValue_ = JsonAdapter::GetValue<float>(data[attack3rdBehaviorJsonKey_], "moveValue_");

		initRotationAngle_ = JsonAdapter::ToObject<Vector3>(data[attack3rdBehaviorJsonKey_]["initRotationAngle_"]);
	}

	// keyframeの初期化
	InitCatmullRom();

	// particle読み込み
	particleSystem_->LoadEmitter("groundEffectEmitter", "groundEffectEmitter");
}

void BodyThirdAttackBehavior::Execute(BasePlayerParts* parts) {

	// 前半攻撃処理
	{
		// 後ろに引く
		FirstHalhUpdateMoveBack(parts);
		FirstHalhUpdateRotationBack(parts);

		// 時間経過を待つ
		FirstHalhWaitMoveTime();

		// 前に行く
		FirstHalhUpdateMoveFront(parts);
		FirstHalhUpdateRotationFront(parts);
	}

	// 後半攻撃処理
	{
		// catmullRom曲線上を移動するようにする
		SecondHalfUpdateMoveCatmullRom(parts);
		// 横を向きながらぐるぐる回転する
		SecondHalfUpdateRotation(parts);
	}
}

void BodyThirdAttackBehavior::FirstHalhUpdateMoveBack(BasePlayerParts* parts) {

	if (!moveBack_->IsStart()) {

		// 開始地点と終了地点を設定
		moveBack_->move_.start = parts->GetTransform().translation;
		moveBack_->move_.end = parts->GetTransform().translation +
			(backwardDirection_ * moveValue_);

		// animation開始
		moveBack_->Start();
	}

	if (moveBack_->IsFinished()) {
		return;
	}

	// 値を補間
	Vector3 translation = parts->GetTransform().translation;
	moveBack_->LerpValue(translation);
	parts->SetTranslate(translation);
}

void BodyThirdAttackBehavior::FirstHalhUpdateRotationBack(BasePlayerParts* parts) {

	if (!backRotation_->IsStart()) {

		// animation開始
		backRotation_->Start();
		// 開始時の回転を記録
		startRotation_ = parts->GetTransform().rotation;
	}

	if (backRotation_->IsFinished()) {
		return;
	}

	// 値を補間
	backRotation_->LerpValue(rotationAngle_);
	// 回転を計算
	Quaternion deltaRotation = IPlayerBehavior::CalRotationAxisAngle(rotationAngle_);
	Quaternion rotation = Quaternion::Normalize(startRotation_ * deltaRotation);
	parts->SetRotate(rotation);
}

void BodyThirdAttackBehavior::FirstHalhUpdateMoveFront(BasePlayerParts* parts) {

	// 時間経過しきったら
	if (enableMoveFront_) {
		if (!moveFront_->IsStart()) {

			// 開始地点と終了地点を設定
			moveFront_->move_.start = parts->GetTransform().translation;
			moveFront_->move_.end = moveBack_->move_.start;

			// animation開始
			moveFront_->Start();
		}
	}

	if (moveFront_->IsFinished()) {
		return;
	}

	// 値を補間
	Vector3 translation = parts->GetTransform().translation;
	moveFront_->LerpValue(translation);
	parts->SetTranslate(translation);
}

void BodyThirdAttackBehavior::FirstHalhUpdateRotationFront(BasePlayerParts* parts) {

	// 時間経過しきったら
	if (enableMoveFront_) {
		if (!frontRotation_->IsStart()) {

			// animation開始
			frontRotation_->Start();
			// 初期値を設定
			frontRotation_->move_.start = rotationAngle_;
		}
	}

	if (frontRotation_->IsFinished()) {
		return;
	}

	// 値を補間
	frontRotation_->LerpValue(rotationAngle_);
	// 回転を計算
	Quaternion deltaRotation = IPlayerBehavior::CalRotationAxisAngle(rotationAngle_);
	Quaternion rotation = Quaternion::Normalize(startRotation_ * deltaRotation);
	parts->SetRotate(rotation);
}

void BodyThirdAttackBehavior::FirstHalhWaitMoveTime() {

	// 後ろに下がりきったら
	if (moveBack_->IsFinished() && !enableMoveFront_) {

		// 時間経過を進める
		moveWaitTimer_ += GameTimer::GetDeltaTime();
		if (moveWaitTimer_ > moveWaitTime_) {

			// 次のanimation開始
			enableMoveFront_ = true;
		}
	}
}

void BodyThirdAttackBehavior::SecondHalfUpdateMoveCatmullRom(BasePlayerParts* parts) {

	// 前に突進し終わったら
	if (moveFront_->IsFinished()) {
		if (!moveKeyframeAnimation_->IsStart()) {

			// keyframeをanimationに設定する
			// 前方ベクトルをもとに回転を計算
			Quaternion rotation = Quaternion::LookRotation(forwardDirection_, Vector3(0.0f, 1.0f, 0.0f));
			Matrix4x4 rotateMatrix = Quaternion::MakeRotateMatrix(rotation);

			Vector3 translation = parts->GetTransform().translation;
			moveKeyframeAnimation_->keyframes_.clear();
			for (uint32_t index = 0; index < moveKeyframes_.size(); ++index) {

				Vector3 keyframe = translation + Vector3::Transform(moveKeyframes_[index], rotateMatrix);
				moveKeyframeAnimation_->keyframes_.emplace_back(keyframe);
			}

			// animation開始
			moveKeyframeAnimation_->Start();
		}
	} else {
		return;
	}

	if (moveKeyframeAnimation_->IsFinished()) {
		return;
	}

	// 値を補完
	Vector3 currentKeyframe{};
	moveKeyframeAnimation_->LerpKeyframeValue(currentKeyframe);
	// 値を設定
	parts->SetTranslate(currentKeyframe);
}

void BodyThirdAttackBehavior::SecondHalfUpdateRotation(BasePlayerParts* parts) {

	if (moveFront_->IsFinished()) {
		if (!rotationAngleZ_->IsStart()) {

			// 初期回転を設定する
			Quaternion initRotation = IPlayerBehavior::CalRotationAxisAngle(initRotationAngle_);
			// 現在の向きも考慮して計算する
			Quaternion rotation = parts->GetTransform().rotation;
			initRotation = Quaternion::Multiply(rotation, initRotation);
			parts->SetRotate(initRotation);

			rotationAngleZ_->Start();
		}
	} else {
		return;
	}

	if (rotationAngleZ_->IsFinished()) {

		if (!emitParticle_) {

			Vector3 forwardDirection = parts->GetTransform().GetForward();

			// effectを発生させる
			Vector3 effectPos = parts->GetTransform().translation;
			effectPos.y = 0.5f;
			particleSystem_->SetTranslate("groundEffectEmitter", effectPos);
			particleSystem_->Emit("groundEffectEmitter");
			emitParticle_ = true;

			// 画面シェイク処理
			followCamera_->StartScreenShake();
		}
		return;
	}

	// Z軸回転させる
	float rotationAngleZ = 0.0f;
	rotationAngleZ_->LerpValue(rotationAngleZ);
	Quaternion rotation = parts->GetTransform().rotation;
	Quaternion newRotation = Quaternion::MakeRotateAxisAngleQuaternion(Vector3(0.0f, 0.0f, 1.0f), rotationAngleZ);
	newRotation = Quaternion::Multiply(rotation, newRotation);
	parts->SetRotate(newRotation);
}

void BodyThirdAttackBehavior::Reset() {

	// 初期化する
	moveBack_->Reset();
	moveFront_->Reset();
	backRotation_->Reset();
	frontRotation_->Reset();
	moveKeyframeAnimation_->Reset();
	rotationAngleZ_->Reset();
	moveWaitTimer_ = 0.0f;
	enableMoveFront_ = false;
	emitParticle_ = false;
}

void BodyThirdAttackBehavior::ImGui() {

	ImGui::PushItemWidth(itemWidth_);

	ImGui::DragFloat("moveWaitTime", &moveWaitTime_, 0.01f);
	ImGui::DragFloat("moveValue", &moveValue_, 0.01f);
	ImGui::DragFloat3("initRotationAngle##BodyThirdAttackBehavior", &initRotationAngle_.x, 0.01f);

	if (ImGui::TreeNode("MoveBack")) {

		moveBack_->ImGui("BodyThirdAttackBehavior_moveBack_");
		backRotation_->ImGui("BodyThirdAttackBehavior_backRotation_");
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("MoveFront")) {

		moveFront_->ImGui("BodyThirdAttackBehavior_moveFront_");
		frontRotation_->ImGui("BodyThirdAttackBehavior_frontRotation_");
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("KeyframeAnimation")) {

		moveKeyframeAnimation_->ImGui("BodyThirdAttackBehavior_moveKeyframeAnimation_");
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("RotationAngleZ")) {

		rotationAngleZ_->ImGui("BodyThirdAttackBehavior_rotationAngleZ_");
		ImGui::TreePop();
	}

	ImGui::PopItemWidth();
}

void BodyThirdAttackBehavior::SaveJson(Json& data) {

	// 値の保存
	moveBack_->ToJson(data[attack3rdBehaviorJsonKey_]["MoveBack"]);
	moveFront_->ToJson(data[attack3rdBehaviorJsonKey_]["MoveFront"]);
	backRotation_->ToJson(data[attack3rdBehaviorJsonKey_]["BackRotation"]);
	frontRotation_->ToJson(data[attack3rdBehaviorJsonKey_]["FrontRotation"]);
	moveKeyframeAnimation_->ToJson(data[attack3rdBehaviorJsonKey_]["MoveKeyframeAnimation"]);
	rotationAngleZ_->ToJson(data[attack3rdBehaviorJsonKey_]["RotationAngleZ"]);

	data[attack3rdBehaviorJsonKey_]["moveValue_"] = moveValue_;
	data[attack3rdBehaviorJsonKey_]["moveWaitTime_"] = moveWaitTime_;
	data[attack3rdBehaviorJsonKey_]["initRotationAngle_"] = JsonAdapter::FromObject<Vector3>(initRotationAngle_);
}

void BodyThirdAttackBehavior::InitCatmullRom() {

	// 4つ分の制御点を確保
	moveKeyframes_.resize(4);

	Json data;
	if (!JsonAdapter::LoadCheck("Player/PartsParameter/Behavior/playerBodyCatmullRom.json", data)) {
		return;
	}

	moveKeyframes_.resize(data["CatmullRomKeyframes"].size());
	for (uint32_t index = 0; index < data["CatmullRomKeyframes"].size(); ++index) {

		std::string key = std::to_string(index);
		moveKeyframes_[index] = JsonAdapter::ToObject<Vector3>(data["CatmullRomKeyframes"][key]);
	}
}

void BodyThirdAttackBehavior::SaveCatmullRom() {

	Json data;

	for (uint32_t index = 0; index < moveKeyframes_.size(); ++index) {

		std::string key = std::to_string(index);
		data["CatmullRomKeyframes"][key] = JsonAdapter::FromObject<Vector3>(moveKeyframes_[index]);
	}

	JsonAdapter::Save("Player/PartsParameter/Behavior/playerBodyCatmullRom", data);
}

void BodyThirdAttackBehavior::EditCatmullRom(
	const Vector3& translation, const Vector3& direction) {

	// 攻撃の時のcatmulRomを線で描画しつつ、デバッグ表示する
	LineRenderer* lineRenderer = LineRenderer::GetInstance();

	ImGui::PushItemWidth(itemWidth_);

	// 制御点の保存
	if (ImGui::Button("Save Keyframe", ImVec2(itemWidth_, 32.0f))) {

		SaveCatmullRom();
	}

	// 制御点の追加
	if (ImGui::Button("Add Keyframe", ImVec2(itemWidth_, 32.0f))) {

		// 最後の点の座標で追加する
		moveKeyframes_.emplace_back(moveKeyframes_.back());
	}

	// 制御点の操作
	// +Z方向を正面とし、後方ベクトル方向によって表示、処理するときは変える
	for (uint32_t index = 0; index < moveKeyframes_.size(); ++index) {

		// 表示する名前
		std::string label = "keyframe" +
			std::to_string(index) + "##BodyThirdAttackBehavior";

		ImGui::DragFloat3(label.c_str(), &moveKeyframes_[index].x, 0.1f);

		ImGui::SameLine();

		// 該当している制御点の削除
		label = "Remove Keyframe" + std::to_string(index) + "##BodyThirdAttackBehavior";
		if (ImGui::Button(label.c_str(), ImVec2(itemWidth_, 32.0f))) {

			moveKeyframes_.erase(moveKeyframes_.begin() + index);
		}
	}

	// keyframeの最初の座標はlocalの原点にする
	moveKeyframes_.front() = Vector3(0.0f, 0.0f, 0.0f);

	// 回転を計算
	Quaternion rotation = Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f));
	Matrix4x4 rotateMatrix = Quaternion::MakeRotateMatrix(rotation);

	// 後方ベクトル方向を向かせて線を繋いで表示
	for (uint32_t i = 0; i < moveKeyframes_.size() - 1; ++i) {

		// playerの座標を考慮した座標にする
		Vector3 start = translation + Vector3::Transform(moveKeyframes_[i], rotateMatrix);
		Vector3 end = translation + Vector3::Transform(moveKeyframes_[i + 1], rotateMatrix);

		lineRenderer->DrawLine3D(start, end, Color::Red());
	}

	const int division = 8;    // 分割数
	const float radius = 0.4f; // 半径
	// 制御点の位置にsphereを描画する
	for (uint32_t i = 0; i < moveKeyframes_.size(); ++i) {

		Vector3 center = translation + Vector3::Transform(moveKeyframes_[i], rotateMatrix);
		lineRenderer->DrawSphere(division, radius, center, Color::Red());
	}

	ImGui::PopItemWidth();
}

void BodyThirdAttackBehavior::SetBackwardDirection(const Vector3& direction) {

	backwardDirection_ = direction;
	backwardDirection_.y = 0.0f;
	backwardDirection_ = backwardDirection_.Normalize();

	// 前方ベクトルはbackの逆
	forwardDirection_ = Vector3::AnyInit(0.0f);
	forwardDirection_ -= backwardDirection_;
}