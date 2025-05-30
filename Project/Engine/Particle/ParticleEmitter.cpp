#include "ParticleEmitter.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Asset/AssetEditor.h>
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Graphics/Mesh/MeshletBuilder.h>
#include <Engine/Renderer/LineRenderer.h>
#include <Engine/Particle/Creators/ParticleCreator.h>
#include <Engine/Particle/Updater/ParticleUpdater.h>
#include <Game/Time/GameTimer.h>
#include <Lib/Adapter/JsonAdapter.h>

//============================================================================
//	ParticleEmitter classMethods
//============================================================================

void ParticleEmitter::Init(const std::string& name,
	Asset* asset, ID3D12Device* device) {

	asset_ = nullptr;
	asset_ = asset;

	device_ = nullptr;
	device_ = device;

	// 名前を設定
	name_ = name;

	// 初期化処理
	transform_.Init();

	// layout
	leftChildSize_ = ImVec2(320.0f, 182.0f);
	rightChildSize_ = ImVec2(380.0f, 182.0f);
	addButtonSize_ = ImVec2(208.0f, 30.0f);
	dropSize_ = ImVec2(208.0f, 30.0f);
}

void ParticleEmitter::Init(const Json& data, const std::string& name,
	Asset* asset, ID3D12Device* device) {

	asset_ = nullptr;
	asset_ = asset;

	device_ = nullptr;
	device_ = device;

	// 名前を設定
	name_ = name;

	// 初期化処理
	transform_.Init();

	// dataをもとにparticleを作成
	if (data.contains("particleNames") && data["particleNames"].is_object()) {

		// particleを走査して作成する
		for (const auto& [index, fileNameJson] : data["particleNames"].items()) {

			std::string fileName = fileNameJson.get<std::string>();
			std::string fullPath = "Particle/" + fileName;

			Json particleData;
			if (JsonAdapter::LoadCheck(fullPath, particleData)) {
				if (particleData.contains("FileType") &&
					particleData["FileType"] == "Particle") {

					// 作成
					CreateParticle(particleData);

					// 同時発生をデフォにする
					isAllEmit_ = true;
					allEmitTime_ = 1.0f;
				}
			}
		}
	}

	// layout
	leftChildSize_ = ImVec2(320.0f, 182.0f);
	rightChildSize_ = ImVec2(380.0f, 182.0f);
	addButtonSize_ = ImVec2(208.0f, 30.0f);
	dropSize_ = ImVec2(208.0f, 30.0f);
}

void ParticleEmitter::Update(const Matrix4x4& billboardMatrix, bool useGame) {

	// 所持しているparticleのemitterの描画
	if (particleGroups_.empty()) {
		return;
	}

	// 全てのparticleの発生処理
	UpdateAllParticle(useGame);

	// 各particleのemitterを描画
	DrawParticleEmitters(useGame);

	// 各particleを更新する
	UpdateParticles(billboardMatrix, useGame);
}

void ParticleEmitter::EditLayout() {

	ImGui::Begin("ParticleEmitter_EditLayout");

	ImGui::DragFloat2("leftChildSize", &leftChildSize_.x, 0.5f);
	ImGui::DragFloat2("rightChildSize", &rightChildSize_.x, 0.5f);
	ImGui::DragFloat2("addButtonSize", &addButtonSize_.x, 0.5f);
	ImGui::DragFloat2("dropSize", &dropSize_.x, 0.5f);

	ImGui::End();
}

void ParticleEmitter::CreateParticle() {

	// particle追加
	particleGroups_.emplace_back();
	ParticleGroup& group = particleGroups_.back();
	// parameterを初期化
	group.parameter.Init(addParticleNameInputText_.name, addModelName_, asset_);

	// buffer作成
	// matrix
	group.worldMatrixBuffer.CreateStructuredBuffer(device_, kMaxInstanceNum_);
	// material
	group.materialBuffer.CreateStructuredBuffer(device_, kMaxInstanceNum_);

	// 頂点、meshlet生成
	const ResourceMesh resourceMesh = CreateMeshlet(addModelName_);
	// mesh作成
	group.mesh = std::make_unique<EffectMesh>();
	group.mesh->Init(device_, resourceMesh, kMaxInstanceNum_);

	// transferData
	group.transferMaterials.resize(kMaxInstanceNum_);
	group.transferMatrices.resize(kMaxInstanceNum_);

	// particleデータの作成
	ParticleCreator::Create(group.particles, group.parameter);
}

void ParticleEmitter::CreateParticle(const Json& data) {

	// particle追加
	particleGroups_.emplace_back();
	ParticleGroup& group = particleGroups_.back();
	// parameterを初期化
	group.parameter.Init(data, asset_);

	// buffer作成
	// matrix
	group.worldMatrixBuffer.CreateStructuredBuffer(device_, kMaxInstanceNum_);
	// material
	group.materialBuffer.CreateStructuredBuffer(device_, kMaxInstanceNum_);

	// 頂点、meshlet生成
	const ResourceMesh resourceMesh = CreateMeshlet(data["Private"]["modelName_"]);
	// mesh作成
	group.mesh = std::make_unique<EffectMesh>();
	group.mesh->Init(device_, resourceMesh, kMaxInstanceNum_);

	// transferData
	group.transferMaterials.resize(kMaxInstanceNum_);
	group.transferMatrices.resize(kMaxInstanceNum_);

	// particleデータの作成
	ParticleCreator::Create(group.particles, group.parameter);
}

ResourceMesh<EffectMeshVertex> ParticleEmitter::CreateMeshlet(const std::string& modelName) {

	ModelData modelData = asset_->GetModelData(modelName);

	// 頂点、meshlet生成
	MeshletBuilder meshletBuilder{};
	ResourceMesh<EffectMeshVertex> resourceMesh = meshletBuilder.ParseEffectMesh(modelData);

	return resourceMesh;
}

void ParticleEmitter::DrawParticleEmitters(bool useGame) {

	// gameで使うときはemitterを表示しない
	if (useGame) {
		return;
	}

	// 各particleのemitterを形状ごとに描画する
	LineRenderer* lineRenderer = LineRenderer::GetInstance();
	const Color& emitterColor = Color::Red(0.6f);
	for (const auto& group : particleGroups_) {
		switch (group.parameter.emitterShape) {
		case EmitterShapeType::Sphere: {

			const uint32_t kDivision = 6;
			EmitterSphere emitterSphere = group.parameter.emitterSphere;
			lineRenderer->DrawSphere(kDivision, emitterSphere.radius, emitterSphere.center, emitterColor);
			break;
		}
		case EmitterShapeType::Hemisphere: {

			const uint32_t kDivision = 8;
			EmitterHemisphere emitterHemisphere = group.parameter.emitterHemisphere;
			lineRenderer->DrawHemisphere(kDivision, emitterHemisphere.radius,
				emitterHemisphere.center, emitterHemisphere.eulerRotate, emitterColor);
			break;
		}
		case EmitterShapeType::Box: {

			EmitterBox emitterBox = group.parameter.emitterBox;
			lineRenderer->DrawOBB(emitterBox.center, emitterBox.size, emitterBox.eulerRotate, emitterColor);
			break;
		}
		case EmitterShapeType::Cone: {

			const uint32_t kDivision = 12;
			EmitterCone emitterCone = group.parameter.emitterCone;
			lineRenderer->DrawCone(kDivision, emitterCone.baseRadius, emitterCone.topRadius,
				emitterCone.height, emitterCone.center, emitterCone.eulerRotate, emitterColor);
			break;
		}
		}
	}
}

void ParticleEmitter::Emit() {

	for (auto& group : particleGroups_) {

		// 最大インスタンス数以下の時のみ
		if (group.numInstance < kMaxInstanceNum_) {
			// particleを作成
			ParticleCreator::Create(group.particles, group.parameter);
		}
	}
}

void ParticleEmitter::UpdateFrequencyEmit() {

	for (auto& group : particleGroups_) {

		// emitCountが0の時は処理しない
		if (group.parameter.emitCount.GetValue() == 0) {
			return;
		}

		// 補間して発生させる
		if (group.parameter.interpolateEmit) {

			UpdateInterpolateEmit(group);
			return;
		}

		float deltaTime = 0.0f;
		if (group.parameter.useScaledTime) {
			deltaTime = GameTimer::GetScaledDeltaTime();
		} else {
			deltaTime = GameTimer::GetDeltaTime();
		}
		// 経過時間を加算
		group.parameter.frequencyTime += deltaTime;
		// 経過時間が発射時間を超えたら
		if (group.parameter.frequency <= group.parameter.frequencyTime) {

			// 時間を元に戻す
			group.parameter.frequencyTime -= group.parameter.frequency;

			// 最大インスタンス数以下の時のみ
			if (group.numInstance < kMaxInstanceNum_) {
				// particleを作成
				ParticleCreator::Create(group.particles, group.parameter);
			}
		}
	}
}

void ParticleEmitter::SetTranslate(const Vector3& translate) {

	// emitterの形状別で回転を設定
	for (auto& particleGroup : particleGroups_) {
		switch (particleGroup.parameter.emitterShape) {
		case EmitterShapeType::Sphere: {

			particleGroup.parameter.emitterSphere.center = translate;
			break;
		}
		case EmitterShapeType::Hemisphere: {

			particleGroup.parameter.emitterHemisphere.center = translate;
			break;
		}
		case EmitterShapeType::Box: {

			particleGroup.parameter.emitterBox.center = translate;
			break;
		}
		case EmitterShapeType::Cone: {

			particleGroup.parameter.emitterCone.center = translate;
			break;
		}
		}
	}
}

void ParticleEmitter::SetRotate(const Quaternion& rotate) {

	// emitterの形状別で回転を設定
	for (auto& particleGroup : particleGroups_) {
		switch (particleGroup.parameter.emitterShape) {
		case EmitterShapeType::Hemisphere: {

			particleGroup.parameter.emitterHemisphere.rotation = rotate;
			break;
		}
		case EmitterShapeType::Box: {

			particleGroup.parameter.emitterBox.rotation = rotate;
			break;
		}
		case EmitterShapeType::Cone: {

			particleGroup.parameter.emitterCone.rotation = rotate;
			break;
		}
		}
	}
}

void ParticleEmitter::UpdateFrequencyEmit(ParticleGroup& group) {

	// 全て発生させてる時こっちは機能させない
	if (isAllEmit_) {
		return;
	}

	// emitCountが0の時は処理しない
	if (group.parameter.emitCount.GetValue() == 0) {
		return;
	}

	// 補間して発生させる
	if (group.parameter.interpolateEmit) {

		UpdateInterpolateEmit(group);
		return;
	}

	float deltaTime = 0.0f;
	if (group.parameter.useScaledTime) {
		deltaTime = GameTimer::GetScaledDeltaTime();
	} else {
		deltaTime = GameTimer::GetDeltaTime();
	}
	// 経過時間を加算
	group.parameter.frequencyTime += deltaTime;
	// 経過時間が発射時間を超えたら
	if (group.parameter.frequency <= group.parameter.frequencyTime) {

		// 時間を元に戻す
		group.parameter.frequencyTime -= group.parameter.frequency;

		// 最大インスタンス数以下の時のみ
		if (group.numInstance < kMaxInstanceNum_) {
			// particleを作成
			ParticleCreator::Create(group.particles, group.parameter);
		}
	}
}

void ParticleEmitter::UpdateInterpolateEmit(ParticleGroup& group) {

	// 発生間隔
	const float spacing = 0.08f;

	// 現在位置と差分
	const Vector3 nowPos = GetEmitterPos(group.parameter);
	const Vector3 diff = nowPos - group.preEmitterPos;
	const float length = Vector3::Length(diff);

	// 動いていなければ処理を止める
	if (length < spacing || length < FLT_EPSILON) {
		group.preEmitterPos = nowPos;
		return;
	}

	// 補間個数と方向を計算
	const int emitCount = static_cast<int>(length / spacing);
	const Vector3 direction = diff / length;

	// 等間隔で発生
	for (int i = 1; i <= emitCount; ++i) {

		// 座標を計算して設定
		const Vector3 pos = group.preEmitterPos + direction * spacing * static_cast<float>(i);
		SetEmitterPos(group, pos);

		// 最大数に到達していなければ発生させる
		if (group.numInstance < kMaxInstanceNum_) {

			ParticleCreator::Create(group.particles, group.parameter);
		}
	}

	// 座標を更新、設定
	SetEmitterPos(group, nowPos);
	group.preEmitterPos = nowPos;
}

void ParticleEmitter::UpdateParticles(const Matrix4x4& billboardMatrix, bool useGame) {

	// 所持しているparticleをすべて更新する
	for (auto& group : particleGroups_) {

#ifdef _DEBUG
		if (!useGame) {
			// emit処理
			UpdateFrequencyEmit(group);
		}
#endif // _DEBUG

		auto& particles = group.particles;
		// 各particleの更新
		uint32_t index = 0;
		for (auto it = particles.begin(); it != particles.end();) {

			// 寿命が無くなったparticleは削除する
			if (it->parameter.lifeTime.value < it->currentTime) {
				it = particles.erase(it);
				continue;
			}

			// particleを更新する
			ParticleUpdater::Update(*it, group.parameter, billboardMatrix);

			// bufferに渡すデータを更新
			// matrix
			group.transferMatrices[index] = it->transform.matrix.world;
			// material
			group.transferMaterials[index].SetMaterial(it->material);

			// イテレータをインクリメント
			++it;
			++index;
		}

		// instance数を更新
		group.numInstance = static_cast<uint32_t>(group.particles.size());
		// bufferを更新
		group.materialBuffer.TransferVectorData(group.transferMaterials, group.numInstance);
		group.worldMatrixBuffer.TransferVectorData(group.transferMatrices, group.numInstance);
	}
}

void ParticleEmitter::UpdateAllParticle(bool useGame) {

	if (useGame) {
		return;
	}

#ifdef _DEBUG
	if (!isAllEmit_) {
		return;
	}

	// 時間が過ぎたら全て発生させる
	allEmitCurrentTimer_ += GameTimer::GetDeltaTime();
	if (allEmitCurrentTimer_ > allEmitTime_) {
		for (auto& group : particleGroups_) {

			// 最大インスタンス数以下の時のみ
			if (group.numInstance < kMaxInstanceNum_) {

				// particleを作成し発生
				ParticleCreator::Create(group.particles, group.parameter);
			}
		}

		// timerリセット処理
		allEmitCurrentTimer_ = 0.0f;
	}
#endif // _DEBUG
}

void ParticleEmitter::ImGui() {

	EditLayout();

	// layout
	ImGui::BeginGroup();

	// emitterの名前を表示
	ImGui::SeparatorText(("EmitterName: " + name_).c_str());
	// emitterの保存処理
	SaveEmitter();
	// 全てのparticleを同時に発生させる
	EmitAllParticle();

	// particle追加処理
	ImGui::BeginChild("AddChild##ParticleEmitter", leftChildSize_, true);
	ImGui::SeparatorText("Add Particle");

	AddParticle();
	LoadParticle();
	ImGui::EndChild();

	// 横並びにする
	ImGui::SameLine();

	// particle選択処理
	ImGui::BeginChild("SelectChild##ParticleEmitter", rightChildSize_, true);
	ImGui::SeparatorText("Select Particle");

	SelectParticle();
	ImGui::EndChild();

	// particleの値操作処理
	ImGui::BeginChild("EditChild##ParticleEmitter");
	ImGui::SeparatorText("Edit Particle");

	EditParticle();
	ImGui::EndChild();

	ImGui::EndGroup();
}

void ParticleEmitter::AddParticle() {

	ImGui::PushID("AddParticlerRow");

	if (ImGui::InputTextWithHint("##ParticleName", "ParticleName",
		addParticleNameInputText_.nameBuffer, IM_ARRAYSIZE(addParticleNameInputText_.nameBuffer))) {

		// 名前を保持
		addParticleNameInputText_.name = addParticleNameInputText_.nameBuffer;
	}

	// ドロップ処理
	if (addModelName_.empty()) {

		// modelをドラッグアンドドロップで取得
		ImGui::Button("Drop model", dropSize_);
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payloadDataId = ImGui::AcceptDragDropPayload(AssetEditor::kDragPayloadId)) {

				auto* payloadModelData = static_cast<AssetEditor::DragPayload*>(payloadDataId->Data);
				// model以外は受け付けない
				if (payloadModelData->type == AssetEditor::PendingType::Model) {

					// modelの名前を保存
					addModelName_ = payloadModelData->name;
				}
			}
			ImGui::EndDragDropTarget();
		}
	} else {

		if (ImGui::Button("Reset", ImVec2(addButtonSize_.x / 2.0f, addButtonSize_.y))) {

			// modelの名前をリセットする
			addModelName_.clear();
		}
		ImGui::SameLine();

		// model名を表示
		ImGui::Text((":" + addModelName_).c_str());
	}

	if (ImGui::Button("Add", addButtonSize_)) {

		// 入力がないときは追加不可
		if (!addParticleNameInputText_.name.empty() &&
			!addModelName_.empty()) {

			// particle作成
			CreateParticle();
			// 入力リセット
			addParticleNameInputText_.Reset();
			addModelName_.clear();
		}
	}

	ImGui::PopID();
}

void ParticleEmitter::LoadParticle() {

	ImGui::PushID("LoadParticlerRow");

	ImGui::Button("Load", addButtonSize_);
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payloadDataId = ImGui::AcceptDragDropPayload(AssetEditor::kDragPayloadId)) {

			auto* payloadJsonData = static_cast<AssetEditor::DragPayload*>(payloadDataId->Data);
			// .json以外は受け付けない
			if (payloadJsonData->type == AssetEditor::PendingType::None) {

				// 読み込み処理
				Json data;
				std::string loadName = "Particle/" + std::string(payloadJsonData->name) + ".json";
				if (JsonAdapter::LoadCheck(loadName, data)) {

					// typeがparticleじゃなければ作成できない
					if (data.contains("FileType")) {
						if (data["FileType"] == "Particle") {

							// Particleなので作成する
							CreateParticle(data);
						}
					}
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::PopID();
}

void ParticleEmitter::SelectParticle() {

	if (particleGroups_.empty()) {
		ImGui::TextDisabled("Particle empty...");
		return;
	}

	for (int i = 0; i < static_cast<int>(particleGroups_.size()); ++i) {

		std::string label = particleGroups_[i].parameter.GetParticleName() + "##" + std::to_string(i);
		bool selected = (currentSelectIndex_.has_value() && currentSelectIndex_.value() == i);
		if (ImGui::Selectable(label.c_str(), selected)) {

			currentSelectIndex_ = i;
		}
		if (selected) {
			ImGui::SetItemDefaultFocus();
		}
	}
}

void ParticleEmitter::EditParticle() {

	// 選択されていないときは何もしない
	if (!currentSelectIndex_.has_value()) {
		return;
	}

	// parameter操作
	ImGui::Text("numInstance: %d / %d, ",
		particleGroups_[currentSelectIndex_.value()].numInstance, kMaxInstanceNum_);
	ImGui::SameLine();
	ImGui::Text("particleSize: %d", particleGroups_[currentSelectIndex_.value()].particles.size());
	// particle保存処理
	SaveParticle();
	ImGui::SameLine();
	// particle削除処理
	RemoveParticle();

	if (currentSelectIndex_.has_value()) {

		particleGroups_[currentSelectIndex_.value()].parameter.ImGui();
	}
}

void ParticleEmitter::SaveEmitter() {

	if (particleGroups_.empty()) {

		ImGui::Text("haven't particle...");
		return;
	}

	if (ImGui::Button("Save Emitter", addButtonSize_)) {

		showEmitterPopup_ = true;
	}

	if (showEmitterPopup_) {

		ImGui::OpenPopup("Save Emitter");
	}

	if (ImGui::BeginPopupModal("Save Emitter", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::Text("Json/ParticleEmitter/%s", emitterSave_.nameBuffer);
		ImGui::InputText("##JsonFilename", emitterSave_.nameBuffer, IM_ARRAYSIZE(emitterSave_.nameBuffer));

		// Save ボタン
		if (ImGui::Button("Save")) {

			emitterSave_.name = "ParticleEmitter/" + std::string(emitterSave_.nameBuffer);
			if (!emitterSave_.name.empty()) {

				// ポップアップを閉じる
				showEmitterPopup_ = false;
				// 保存処理、すべてのparticleを走査し、保存する
				// emitterはParticleEmitter/に保存し、
				// particleはParticle/に保存する
				// emitterがparticleのファイル名をjsonに保存する
				Json data;
				data["FileType"] = "ParticleEmitter";
				std::vector<std::string> particleNames(particleGroups_.size());

				for (uint32_t index = 0; index < particleGroups_.size(); ++index) {

					// particleの保存名を取得
					particleNames[index] = std::string(emitterSave_.nameBuffer) + "_" + particleGroups_[index].parameter.GetParticleName() + ".json";

					// particleを保存する
					particleGroups_[index].parameter.SaveJson("Particle/" + particleNames[index]);

					data["particleNames"][std::to_string(index)] = particleNames[index];
				}
				// emitterを保存する
				JsonAdapter::Save(emitterSave_.name, data);

				emitterSave_.Reset();

				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::SameLine();

		// Cancel
		if (ImGui::Button("Cancel")) {

			emitterSave_.Reset();

			// ポップアップを閉じる
			showEmitterPopup_ = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void ParticleEmitter::EmitAllParticle() {

	ImGui::Checkbox("isAllEmit", &isAllEmit_);

	ImGui::SameLine();

	ImGui::DragFloat("allEmitTime", &allEmitTime_, 0.01f);
}

void ParticleEmitter::SaveParticle() {

	if (ImGui::Button("Save Particle", addButtonSize_)) {

		showParticlePopup_ = true;
	}

	if (showParticlePopup_) {

		ImGui::OpenPopup("Save Particle");
	}

	if (ImGui::BeginPopupModal("Save Particle", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::Text("Json/Particle/%s", particleSave_.nameBuffer);
		ImGui::InputText("##JsonFilename", particleSave_.nameBuffer, IM_ARRAYSIZE(particleSave_.nameBuffer));

		// Save ボタン
		if (ImGui::Button("Save")) {

			particleSave_.name = "Particle/" + std::string(particleSave_.nameBuffer);
			if (!particleSave_.name.empty()) {

				// ポップアップを閉じる
				showParticlePopup_ = false;
				// 保存処理
				particleGroups_[currentSelectIndex_.value()].parameter.SaveJson(particleSave_.name);

				particleSave_.Reset();

				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::SameLine();

		// Cancel
		if (ImGui::Button("Cancel")) {

			particleSave_.Reset();

			// ポップアップを閉じる
			showParticlePopup_ = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void ParticleEmitter::RemoveParticle() {

	if (!currentSelectIndex_.has_value() || particleGroups_.empty()) {
		return;
	}

	if (ImGui::Button("Remove Particle", addButtonSize_)) {

		const int removeIndex = currentSelectIndex_.value();

		// 範囲チェック
		if (0 <= removeIndex && removeIndex < static_cast<int>(particleGroups_.size())) {

			// vectorから削除
			particleGroups_.erase(particleGroups_.begin() + removeIndex);
			currentSelectIndex_ = std::nullopt;
		}
	}
}

void ParticleEmitter::SetEmitterPos(ParticleGroup& group, const Vector3& pos) {

	switch (group.parameter.emitterShape) {
	case EmitterShapeType::Sphere: {

		group.parameter.emitterSphere.center = pos;
		break;
	}
	case EmitterShapeType::Hemisphere: {

		group.parameter.emitterHemisphere.center = pos;
		break;
	}
	case EmitterShapeType::Box: {

		group.parameter.emitterBox.center = pos;
		break;
	}
	case EmitterShapeType::Cone: {

		group.parameter.emitterCone.center = pos;
		break;
	}
	}
}

Vector3 ParticleEmitter::GetEmitterPos(const ParticleParameter& parameter) const {

	Vector3 emitterPos{};
	switch (parameter.emitterShape) {
	case EmitterShapeType::Sphere: {

		emitterPos = parameter.emitterSphere.center;
		break;
	}
	case EmitterShapeType::Hemisphere: {

		emitterPos = parameter.emitterHemisphere.center;
		break;
	}
	case EmitterShapeType::Box: {

		emitterPos = parameter.emitterBox.center;
		break;
	}
	case EmitterShapeType::Cone: {

		emitterPos = parameter.emitterCone.center;
		break;
	}
	}

	return emitterPos;
}

void ParticleEmitter::InputTextValue::Reset() {

	// 入力をリセット
	nameBuffer[0] = '\0';
	name.clear();
}

void EffectMaterial::Init() {

	color = Color::White();
	useNoiseTexture = false;
	useVertexColor = false;
	textureAlphaReference = 0.5f;
	noiseTextureAlphaReference = 0.0f;
	emissiveIntensity = 0.0f;
	emissionColor = Vector3::AnyInit(1.0f);
	edgeSize = 0.8f;
	edgeColor = Color::White();
	edgeEmissiveIntensity = 0.0f;
	edgeEmissionColor = Vector3::AnyInit(1.0f);
	uvTransform = Matrix4x4::MakeIdentity4x4();
}

void EffectMaterial::SetMaterial(const EffectMaterial& material) {

	color = material.color;
	textureIndex = material.textureIndex;
	noiseTextureIndex = material.noiseTextureIndex;
	useNoiseTexture = material.useNoiseTexture;
	useVertexColor = material.useVertexColor;
	samplerType = material.samplerType;
	textureAlphaReference = material.textureAlphaReference;
	noiseTextureAlphaReference = material.noiseTextureAlphaReference;
	emissiveIntensity = material.emissiveIntensity;
	emissionColor = material.emissionColor;
	edgeSize = material.edgeSize;
	edgeColor = material.edgeColor;
	edgeEmissiveIntensity = material.edgeEmissiveIntensity;
	edgeEmissionColor = material.edgeEmissionColor;
	uvTransform = material.uvTransform;
}