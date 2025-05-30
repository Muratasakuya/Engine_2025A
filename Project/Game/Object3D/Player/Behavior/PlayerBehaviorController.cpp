#include "PlayerBehaviorController.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Particle/ParticleSystem.h>
#include <Game/Time/GameTimer.h>

//============================================================================
//	PlayerBehaviorController classMethods
//============================================================================

void PlayerBehaviorController::Init() {

	input_ = nullptr;
	input_ = Input::GetInstance();

	isAcceptedMode_ = false;

	// json適応
	ApplyJson();
}

void PlayerBehaviorController::Update() {

	// behaviorの変更不可、設定不可
	if (isAcceptedMode_) {
		return;
	}

	// 移動更新処理
	UpdateMove();
	// 攻撃更新処理
	UpdateAttack();

	// 待機モーションチェック
	CheckWait();

	// behaviourを設定する
	BehaviourRequest();

	// particleを常に更新する
	ParticleSystem* particleSystem = ParticleSystem::GetInstance();

	// ground
	particleSystem->UpdateEmitter("groundEffectEmitter");
	// hitEffect
	particleSystem->UpdateEmitter("hitEffectEmitter");
	// dash
	particleSystem->UpdateEmitter("dashEffectEmitter");
}

void PlayerBehaviorController::UpdateMove() {

	// 攻撃中はここの操作はできないようにしておく
	bool isAttack = false;
	isAttack |= limits_[PlayerBehaviorType::Attack_1st].isUpdating;
	isAttack |= limits_[PlayerBehaviorType::Attack_2nd].isUpdating;
	isAttack |= limits_[PlayerBehaviorType::Attack_3rd].isUpdating;

	if (isAttack) {
		return;
	}

	// 歩き処理
	MoveWalk();
	// ダッシュ処理
	MoveDash();
}

void PlayerBehaviorController::UpdateAttack() {

	// 1段目攻撃処理
	FirstAttack();
	// 2段目攻撃処理
	SecondAttack();
	// 3段目攻撃処理
	ThirdAttack();
}

void PlayerBehaviorController::MoveWalk() {

	// WASDどれか押していたら
	bool inputKey =
		input_->PushKey(DIK_W) ||
		input_->PushKey(DIK_A) ||
		input_->PushKey(DIK_S) ||
		input_->PushKey(DIK_D);

	// 歩きを設定する
	if (inputKey) {

		moveBehaviour_ = PlayerBehaviorType::Walk;
	} else {
		// 入力がなければ削除する
		if (CheckCurrentBehaviors({ PlayerBehaviorType::Walk }, MatchType::All)) {

			currentMoveBehaviours_.erase(PlayerBehaviorType::Walk);
		}
	}
}

void PlayerBehaviorController::MoveDash() {

	// WASDどれか押していたら
	bool inputKey =
		input_->PushKey(DIK_W) ||
		input_->PushKey(DIK_A) ||
		input_->PushKey(DIK_S) ||
		input_->PushKey(DIK_D);

	// ダッシュを設定する
	if (inputKey) {
		// 右クリックでダッシュ
		if (input_->PushMouseRight()) {

			moveBehaviour_ = PlayerBehaviorType::Dash;
			// ダッシュになったら歩き操作は消す
			currentMoveBehaviours_.erase(PlayerBehaviorType::Walk);
		} else {
			// ダッシュ中に右クリックを離したらダッシュを止める
			if (CheckCurrentBehaviors({ PlayerBehaviorType::Dash }, MatchType::All)) {

				currentMoveBehaviours_.erase(PlayerBehaviorType::Dash);
			}
		}
	}
}

void PlayerBehaviorController::FirstAttack() {

	// 他の攻撃中はここの操作はできないようにしておく
	bool isAttack = false;
	isAttack |= limits_[PlayerBehaviorType::Attack_2nd].isUpdating;
	isAttack |= limits_[PlayerBehaviorType::Attack_3rd].isUpdating;

	if (isAttack) {
		return;
	}

	// 1段目の攻撃を設定する
	// 1段目攻撃中は受け付けない
	if (!limits_[PlayerBehaviorType::Attack_1st].isUpdating) {
		// 待機か、歩き中
		if (CheckCurrentBehaviors({
			PlayerBehaviorType::Wait,
			PlayerBehaviorType::Walk }, MatchType::Any)) {
			// スペースキーで 1段目の攻撃
			if (input_->TriggerKey(DIK_SPACE)) {

				moveBehaviour_ = PlayerBehaviorType::Attack_1st;
				limits_[PlayerBehaviorType::Attack_1st].Reset();

				// 更新状態にする
				limits_[PlayerBehaviorType::Attack_1st].isUpdating = true;
				// 現在有効な操作をすべて削除する
				currentMoveBehaviours_.clear();
			}
		}
	}

	// 設定されているとき
	if (CheckCurrentBehaviors({ PlayerBehaviorType::Attack_1st }, MatchType::All)) {

		// 経過時間を進める
		limits_[PlayerBehaviorType::Attack_1st].UpdateElapseTime();
		// 最大時間になったらリセットし、攻撃を止める
		if (limits_[PlayerBehaviorType::Attack_1st].isReached) {

			limits_[PlayerBehaviorType::Attack_1st].isUpdating = false;
			currentMoveBehaviours_.erase(PlayerBehaviorType::Attack_1st);
		}
	}
}

void PlayerBehaviorController::SecondAttack() {

	// 2段目の攻撃を設定する
	// 2段目攻撃中は受け付けない
	if (!limits_[PlayerBehaviorType::Attack_2nd].isUpdating) {
		// 1段目攻撃中
		if (CheckCurrentBehaviors({ PlayerBehaviorType::Attack_1st }, MatchType::All)) {
			// スペースキーを押したら2段目の攻撃を予約する
			if (input_->TriggerKey(DIK_SPACE)) {

				limits_[PlayerBehaviorType::Attack_2nd].isReserve = true;
				limits_[PlayerBehaviorType::Attack_2nd].Reset();
			}
		}
	}

	// 設定されているとき
	if (CheckCurrentBehaviors({ PlayerBehaviorType::Attack_2nd }, MatchType::All)) {

		// 経過時間を進める
		limits_[PlayerBehaviorType::Attack_2nd].UpdateElapseTime();
		// 最大時間になったらリセットし、攻撃を止める
		if (limits_[PlayerBehaviorType::Attack_2nd].isReached) {

			limits_[PlayerBehaviorType::Attack_2nd].isUpdating = false;
			currentMoveBehaviours_.erase(PlayerBehaviorType::Attack_2nd);
		}
	}

	// 1段目の攻撃が完全に終了したら
	if (limits_[PlayerBehaviorType::Attack_1st].isReached) {
		// 予約されていれば
		if (limits_[PlayerBehaviorType::Attack_2nd].isReserve) {

			// 攻撃を開始する
			moveBehaviour_ = PlayerBehaviorType::Attack_2nd;

			// 更新状態にする
			limits_[PlayerBehaviorType::Attack_2nd].isUpdating = true;
			// 予約解除
			limits_[PlayerBehaviorType::Attack_2nd].isReserve = false;
			// 現在有効な操作をすべて削除する
			currentMoveBehaviours_.clear();
		}
	}
}

void PlayerBehaviorController::ThirdAttack() {

	// 3段目の攻撃を設定する
	// 3段目攻撃中は受け付けない
	if (!limits_[PlayerBehaviorType::Attack_3rd].isUpdating) {
		// 1段目攻撃中
		if (CheckCurrentBehaviors({ PlayerBehaviorType::Attack_2nd }, MatchType::All)) {
			// スペースキーを押したら3段目の攻撃を予約する
			if (input_->TriggerKey(DIK_SPACE)) {

				limits_[PlayerBehaviorType::Attack_3rd].isReserve = true;
				limits_[PlayerBehaviorType::Attack_3rd].Reset();
			}
		}
	}

	// 設定されているとき
	if (CheckCurrentBehaviors({ PlayerBehaviorType::Attack_3rd }, MatchType::All)) {

		// 経過時間を進める
		limits_[PlayerBehaviorType::Attack_3rd].UpdateElapseTime();
		// 最大時間になったらリセットし、攻撃を止める
		if (limits_[PlayerBehaviorType::Attack_3rd].isReached) {

			limits_[PlayerBehaviorType::Attack_3rd].isUpdating = false;
			currentMoveBehaviours_.erase(PlayerBehaviorType::Attack_3rd);
		}
	}

	// 1段目の攻撃が完全に終了したら
	if (limits_[PlayerBehaviorType::Attack_2nd].isReached) {
		// 予約されていれば
		if (limits_[PlayerBehaviorType::Attack_3rd].isReserve) {

			// 攻撃を開始する
			moveBehaviour_ = PlayerBehaviorType::Attack_3rd;

			// 更新状態にする
			limits_[PlayerBehaviorType::Attack_3rd].isUpdating = true;
			// 予約解除
			limits_[PlayerBehaviorType::Attack_3rd].isReserve = false;
			// 現在有効な操作をすべて削除する
			currentMoveBehaviours_.clear();
		}
	}
}

void PlayerBehaviorController::CheckWait() {

	// 攻撃中はここの操作はできないようにしておく
	bool isAttack = false;
	isAttack |= limits_[PlayerBehaviorType::Attack_1st].isUpdating;
	isAttack |= limits_[PlayerBehaviorType::Attack_2nd].isUpdating;
	isAttack |= limits_[PlayerBehaviorType::Attack_3rd].isUpdating;

	if (isAttack) {
		return;
	}

	// 入力が何もないか、moveBehaviour_に何も値が入っていないか
	bool inputKey =
		!input_->PushKey(DIK_W) &&
		!input_->PushKey(DIK_A) &&
		!input_->PushKey(DIK_S) &&
		!input_->PushKey(DIK_D);

	// 待ちモーションを開始させる
	if (inputKey && !moveBehaviour_.has_value()) {

		moveBehaviour_ = PlayerBehaviorType::Wait;
	} else {
		// 何か操作があれば削除する
		if (CheckCurrentBehaviors({ PlayerBehaviorType::Wait }, MatchType::All)) {

			currentMoveBehaviours_.erase(PlayerBehaviorType::Wait);
		}
	}
}

void PlayerBehaviorController::BehaviourRequest() {

	// 行動が追加されていれば設定する
	if (moveBehaviour_.has_value()) {

		// 同じ値はセットできないようにする
		if (!CheckCurrentBehaviors({ *moveBehaviour_ }, MatchType::All)) {

			currentMoveBehaviours_.insert(*moveBehaviour_);
		}
		// behaviourリセット
		moveBehaviour_ = std::nullopt;
	}
}

void PlayerBehaviorController::ImGui() {

	ImGui::SeparatorText("BehaviorSet");

	ImGui::Text(std::format("isAcceptedMode: {}", isAcceptedMode_).c_str());

	// behaviorを更新できないようにする
	if (ImGui::Button("AcceptedMode", ImVec2(itemWidth_, 32.0f))) {

		isAcceptedMode_ = !isAcceptedMode_;
		currentMoveBehaviours_.clear();
	}

	// 値を保存
	if (ImGui::Button("Save Json", ImVec2(itemWidth_, 32.0f))) {

		SaveJson();
	}

	// 現在有効なbehaviorを表示
	if (!currentMoveBehaviours_.empty()) {

		std::string behaviourText;
		for (auto behaviour : currentMoveBehaviours_) {
			switch (behaviour) {
			case PlayerBehaviorType::Wait:       behaviourText += "Wait, "; break;
			case PlayerBehaviorType::Walk:       behaviourText += "Walk, "; break;
			case PlayerBehaviorType::Dash:       behaviourText += "Dash, "; break;
			case PlayerBehaviorType::Attack_1st: behaviourText += "Attack_1st, "; break;
			case PlayerBehaviorType::DashAttack: behaviourText += "DashAttack, "; break;
			case PlayerBehaviorType::Attack_2nd: behaviourText += "Attack_2nd, "; break;
			case PlayerBehaviorType::Attack_3rd: behaviourText += "Attack_3rd, "; break;
			case PlayerBehaviorType::Parry:      behaviourText += "Parry, "; break;
			}
		}

		if (!behaviourText.empty()) {

			behaviourText.pop_back();
			behaviourText.pop_back();
		}

		ImGui::Text("%s", std::format("Current Behaviors: {}", behaviourText).c_str());
	}

	// 止まっている状態から攻撃...1段目
	if (ImGui::CollapsingHeader("Attack_1st")) {

		if (ImGui::Button("Execute##Attack_1st", ImVec2(itemWidth_, 32.0f))) {

			moveBehaviour_ = PlayerBehaviorType::Attack_1st;

			// 更新状態にする
			limits_[PlayerBehaviorType::Attack_1st].isUpdating = true;
			// 現在有効な操作をすべて削除する
			currentMoveBehaviours_.clear();
		}
		limits_[PlayerBehaviorType::Attack_1st].ImGui("Attack_1st");
	}

	// 2段目の攻撃
	if (ImGui::CollapsingHeader("Attack_2nd")) {

		if (ImGui::Button("Execute##Attack_2nd", ImVec2(itemWidth_, 32.0f))) {

			moveBehaviour_ = PlayerBehaviorType::Attack_2nd;

			// 更新状態にする
			limits_[PlayerBehaviorType::Attack_2nd].isUpdating = true;
			// 現在有効な操作をすべて削除する
			currentMoveBehaviours_.clear();
		}
		limits_[PlayerBehaviorType::Attack_2nd].ImGui("Attack_2nd");
	}

	// 3段目の攻撃
	if (ImGui::CollapsingHeader("Attack_3rd")) {

		if (ImGui::Button("Execute##Attack_3rd", ImVec2(itemWidth_, 32.0f))) {

			moveBehaviour_ = PlayerBehaviorType::Attack_3rd;

			// 更新状態にする
			limits_[PlayerBehaviorType::Attack_3rd].isUpdating = true;
			// 現在有効な操作をすべて削除する
			currentMoveBehaviours_.clear();
		}
		limits_[PlayerBehaviorType::Attack_3rd].ImGui("Attack_3rd");
	}
}

bool PlayerBehaviorController::CheckCurrentBehaviors(
	const std::initializer_list<PlayerBehaviorType> behaviours, MatchType matchType) {

	// 全部一致
	if (matchType == MatchType::All) {
		for (auto state : behaviours) {
			if (currentMoveBehaviours_.find(state) == currentMoveBehaviours_.end()) {

				// 1つもない場合
				return false;
			}
		}
		// 全部あった場合
		return true;
	}
	// どれか一致
	else if (matchType == MatchType::Any) {
		for (auto state : behaviours) {
			if (currentMoveBehaviours_.find(state) != currentMoveBehaviours_.end()) {

				// 1つが一致したとき
				return true;
			}
		}
		// 1つもない場合
		return false;
	}
	return false;
}

const std::unordered_set<PlayerBehaviorType>& PlayerBehaviorController::GetCurrentBehaviours() const {

	return currentMoveBehaviours_;
}

void PlayerBehaviorController::LimitTime::UpdateElapseTime() {

	elapsed += GameTimer::GetDeltaTime();

	// 経過時間チェック
	if (elapsed > limit) {

		// 時間を過ぎた
		isReached = true;
	}
}

void PlayerBehaviorController::LimitTime::Reset() {

	elapsed = 0.0f;
	isReached = false;
}

void PlayerBehaviorController::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Player/Controller/behaviorController.json", data)) {
		return;
	}

	limits_[PlayerBehaviorType::Attack_1st].ApplyJson(data, "Attack_1st");
	limits_[PlayerBehaviorType::Attack_2nd].ApplyJson(data, "Attack_2nd");
	limits_[PlayerBehaviorType::Attack_3rd].ApplyJson(data, "Attack_3rd");
}

void PlayerBehaviorController::SaveJson() {

	Json data;

	limits_[PlayerBehaviorType::Attack_1st].SaveJson(data, "Attack_1st");
	limits_[PlayerBehaviorType::Attack_2nd].SaveJson(data, "Attack_2nd");
	limits_[PlayerBehaviorType::Attack_3rd].SaveJson(data, "Attack_3rd");

	JsonAdapter::Save("Player/Controller/behaviorController.json", data);
}

void PlayerBehaviorController::LimitTime::ImGui(const std::string& label) {

	ImGui::PushItemWidth(itemWidth_);

	ImGui::Text(std::format("isUpdating: {}", isUpdating).c_str());

	ImGui::DragFloat(("limit" + label).c_str(), &limit, 0.01f);

	ImGui::PopItemWidth();
}

void PlayerBehaviorController::LimitTime::ApplyJson(
	const Json& data, const std::string& key) {

	if (!data.contains(key)) {
		return;
	}

	limit = JsonAdapter::GetValue<float>(data[key], "limit");
}

void PlayerBehaviorController::LimitTime::SaveJson(
	Json& data, const std::string& key) {

	data[key]["limit"] = limit;
}