#include "ParticleUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Particle/ParticleEmitter.h>
#include <Game/Time/GameTimer.h>

//============================================================================
//	ParticleUpdater classMethods
//============================================================================

void ParticleUpdater::Update(ParticleData& particle,
	const ParticleParameter& parameter, const Matrix4x4& billboardMatrix) {

	// 時間の更新処理
	float deltaTime = 0.0f;
	UpdateTime(deltaTime, particle, parameter);

	// イージング後の速度を計算
	particle.easedVelocity = particle.velocity * EasedValue(parameter.moveEasingType, particle.progress);
	// SRTの更新処理
	UpdateScale(particle, parameter);
	UpdateRotate(deltaTime, particle, parameter);
	UpdateTranslate(deltaTime, particle, parameter);
	// worldMatrixの更新処理
	UpdateMatrix(particle, parameter, billboardMatrix);

	// materialの更新
	UpdateMaterial(particle, parameter);
}

void ParticleUpdater::UpdateTime(float& deltaTime, ParticleData& particle, const ParticleParameter& parameter) {

	// deltaTimeの分岐
	if (parameter.useScaledTime) {
		deltaTime = GameTimer::GetScaledDeltaTime();
	} else {
		deltaTime = GameTimer::GetDeltaTime();
	}
	// 時間を進める
	particle.currentTime += deltaTime;

	// 進行度計算
	particle.progress = std::clamp(particle.currentTime / particle.parameter.lifeTime.value, 0.0f, 1.0f);
}

void ParticleUpdater::UpdateScale(ParticleData& particle, [[maybe_unused]] const ParticleParameter& parameter) {

	// targetに向けて補間する
	particle.transform.scale = Vector3::Lerp(
		particle.parameter.startScale.value,
		particle.parameter.targetScale.value, EasedValue(parameter.scaleEasingType, particle.progress));
}

void ParticleUpdater::UpdateRotate(float deltaTime, ParticleData& particle, const ParticleParameter& parameter) {

	// 進行方向に回転を設定する
	if (parameter.moveToDirection &&
		parameter.billboardType == ParticleBillboardType::None ||
		parameter.billboardType == ParticleBillboardType::YAxis) {

		// 進行方向を取得
		Vector3 direction = particle.easedVelocity.Normalize();
		particle.transform.eulerRotate.y = std::atan2(direction.x, direction.z);
		// 横軸方向の長さを求める
		float horizontalLength = std::sqrtf(direction.x * direction.x + direction.z * direction.z);
		// X軸周りの角度(θx)
		particle.transform.eulerRotate.x = std::atan2(-direction.y, horizontalLength);
	} else {

		// 回転量をtargetに向けて補間
		Vector3 rotationMultiplier = Vector3::Lerp(
			particle.parameter.startRotationMultiplier.value,
			particle.parameter.targetRotationMultiplier.value, EasedValue(parameter.rotationEasingType, particle.progress));
		// 回転量を足す
		particle.transform.eulerRotate += rotationMultiplier * deltaTime;
	}
}

void ParticleUpdater::UpdateTranslate(float deltaTime,
	ParticleData& particle, const ParticleParameter& parameter) {

	// 壁に反射するかどうか
	if (parameter.reflectGround) {

		Vector3 newPosition = particle.transform.translation + particle.easedVelocity * deltaTime;

		// 衝突判定
		if (newPosition.y <= particle.parameter.reflectFace.y && particle.velocity.y < 0.0f) {

			// 反射処理
			particle.velocity = Vector3::Reflect(particle.velocity, Vector3(0.0f, 1.0f, 0.0f)) * particle.parameter.restitution.value;

			// 反射位置よりも進めないように調整
			newPosition.y = particle.parameter.reflectFace.y;
		}

		// 更新された位置に適用
		particle.transform.translation = newPosition;
	} else {

		// 座標を加算
		particle.transform.translation += particle.easedVelocity * deltaTime;
	}

	// 重力の適応
	Vector3 gravityEffect =
		particle.parameter.gravityDirection.value * particle.parameter.gravityStrength.value * deltaTime;
	particle.velocity += gravityEffect;
}

void ParticleUpdater::UpdateMatrix(ParticleData& particle,
	const ParticleParameter& parameter, const Matrix4x4& billboardMatrix) {

	// STの行列計算
	Matrix4x4 scaleMatrix = Matrix4x4::MakeScaleMatrix(particle.transform.scale);
	Matrix4x4 translateMatrix = Matrix4x4::MakeTranslateMatrix(particle.transform.translation);

	// worldMatrixの更新処理、billboardTypeで分岐
	// 全軸
	switch (parameter.billboardType) {
	case ParticleBillboardType::None: {

		// billboard無し
		particle.transform.matrix.world = Matrix4x4::MakeAffineMatrix(
			particle.transform.scale, particle.transform.eulerRotate, particle.transform.translation);
		break;
	}
	case ParticleBillboardType::All: {

		// 全軸
		particle.transform.matrix.world = scaleMatrix * billboardMatrix * translateMatrix;
		break;
	}
	case ParticleBillboardType::YAxis: {

		// Y軸
		Quaternion billboardRot = Quaternion::FromRotationMatrix(billboardMatrix);
		// 回転行列取得
		Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(particle.transform.eulerRotate);

		Vector3 xAxis = Vector3::TransferNormal(Vector3(1.0f, 0.0f, 0.0f), rotateMatrix);
		Vector3 zAxis = Vector3::TransferNormal(Vector3(0.0f, 0.0f, 1.0f), rotateMatrix);

		// XZだけの回転行列作成
		Vector3 newZ = Vector3::Normalize(zAxis);
		Vector3 newX = Vector3::Normalize(xAxis);
		Vector3 newY = Vector3::Normalize(Vector3::Cross(newZ, newX));
		newX = Vector3::Normalize(Vector3::Cross(newY, newZ));

		// XZの回転行列からquaternionを取得
		Matrix4x4 xzRotMatrix = {
			newX.x, newX.y, newX.z, 0.0f,
			newY.x, newY.y, newY.z, 0.0f,
			newZ.x, newZ.y, newZ.z, 0.0f,
			0.0f,   0.0f,   0.0f,   1.0f };
		Quaternion xzRotation = Quaternion::FromRotationMatrix(xzRotMatrix);

		// Y軸はbillboard、XZはrotation
		Quaternion finalRotation = Quaternion::Multiply(Quaternion::Conjugate(billboardRot), xzRotation);
		finalRotation = Quaternion::Normalize(finalRotation);

		Matrix4x4 finalRotateMatrix = Quaternion::MakeRotateMatrix(finalRotation);
		particle.transform.matrix.world = scaleMatrix * finalRotateMatrix * translateMatrix;
		break;
	}
	}
}

void ParticleUpdater::UpdateMaterial(ParticleData& particle, const ParticleParameter& parameter) {

	// texture情報を渡す
	particle.material.textureIndex = parameter.textureIndex;
	particle.material.noiseTextureIndex = parameter.noiseTextureIndex;
	// sampler情報を渡す
	particle.material.samplerType = static_cast<int32_t>(parameter.uvAdressMode);

	// 色をtargetに向けて補間
	particle.material.color = Color::Lerp(
		particle.parameter.startColor.value,
		particle.parameter.targetColor.value, EasedValue(parameter.colorEasingType, particle.progress));

	// 発光
	// 強度、targetに向けて補間
	particle.material.emissiveIntensity = std::lerp(
		particle.parameter.startEmissiveIntensity.value,
		particle.parameter.targetEmissiveIntensity.value, EasedValue(parameter.emissionEasingType, particle.progress));
	// 色、targetに向けて補間
	particle.material.emissionColor = Vector3::Lerp(
		particle.parameter.startEmissionColor.value,
		particle.parameter.targetEmissionColor.value, EasedValue(parameter.emissionEasingType, particle.progress));

	// alphaReference、targetに向けて補間
	// texture
	particle.material.textureAlphaReference = std::lerp(
		particle.parameter.startTextureAlphaReference.value,
		particle.parameter.targetTextureAlphaReference.value,
		EasedValue(parameter.alphaReferenceEasingType, particle.progress));
	// noiseTextureを使用する場合のみ処理を行う
	particle.material.useNoiseTexture = parameter.useNoiseTexture;
	if (particle.material.useNoiseTexture) {

		// discard値
		particle.material.noiseTextureAlphaReference = std::lerp(
			particle.parameter.startNoiseTextureAlphaReference.value,
			particle.parameter.targetNoiseTextureAlphaReference.value,
			EasedValue(parameter.alphaReferenceEasingType, particle.progress));

		// エッジ処理
		particle.material.edgeSize = std::lerp(
			particle.parameter.startEdgeSize.value,
			particle.parameter.targetEdgeSize.value,
			EasedValue(parameter.edgeEasingType, particle.progress));
		particle.material.edgeColor = Color::Lerp(
			particle.parameter.startEdgeColor.value,
			particle.parameter.targetEdgeColor.value,
			EasedValue(parameter.edgeEasingType, particle.progress));

		// エッジ発光
		particle.material.edgeEmissiveIntensity = std::lerp(
			particle.parameter.startEmissiveIntensity.value,
			particle.parameter.targetEmissiveIntensity.value,
			EasedValue(parameter.edgeEmissionEasingType, particle.progress));
		particle.material.edgeEmissionColor = Vector3::Lerp(
			particle.parameter.startEdgeEmissionColor.value,
			particle.parameter.targetEdgeEmissionColor.value,
			EasedValue(parameter.edgeEmissionEasingType, particle.progress));
	}

	// uvTransformを使用する場合のみ処理する
	if (parameter.useUVTransform) {

		// UVスケール
		Vector3 uvScale = Vector3::Lerp(
			particle.parameter.startUVScale.value,
			particle.parameter.targetUVScale.value, EasedValue(parameter.uvScaleEasingType, particle.progress));
		// UVZ回転
		float uvRotationZ = std::lerp(
			particle.parameter.startUVRotationZ.value,
			particle.parameter.targetUVRotationZ.value, EasedValue(parameter.uvRotationZEasingType, particle.progress));
		// UV座標
		Vector3 uvTranslation = Vector3::Lerp(
			particle.parameter.startUVTranslation.value,
			particle.parameter.targetUVTranslation.value, EasedValue(parameter.uvTranslationEasingType, particle.progress));

		// matrix計算
		particle.material.uvTransform = Matrix4x4::MakeAffineMatrix(
			uvScale, Vector3(0.0f, 0.0f, uvRotationZ), uvTranslation);
	}
}