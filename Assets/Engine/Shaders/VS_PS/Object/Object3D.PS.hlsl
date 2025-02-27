//============================================================================*/
//	include
//============================================================================*/

#include "Object3D.hlsli"

//============================================================================*/
//	Object3D PS
//============================================================================*/

struct Material {

	float4 color;
	int enableLighting;
	int enableHalfLambert;
	int enablePhongReflection;
	int enableBlinnPhongReflection;
	float shadowRate;
	float shininess;
	float3 specularColor;
	float emissiveIntensity;
	float3 emissionColor;
	float4x4 uvTransform;
};

/// ���s����
struct DirectionalLight {

	float4 color;
	float3 direction;
	float intensity;
};

/// �|�C���g���C�g
struct PointLight {
	
	float4 color;
	float3 pos;
	float intensity;
	float radius;
	float decay;
};

/// �X�|�b�g���C�g
struct SpotLight {

	float4 color;
	float3 pos;
	float intensity;
	float3 direction;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
};

// �S�Ẵ��C�g
struct PunctualLight {
	
	DirectionalLight directionalLight;
	PointLight pointLight;
	SpotLight spotLight;
};

// �J����
struct Camera {

	float3 worldPosition;
};

struct PixelShaderOutput {
    
	float4 color : SV_TARGET0;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<PunctualLight> gPunctual : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

Texture2D<float4> gTexture : register(t0);
Texture2D<float3> gShadowTexture : register(t1);
SamplerState gSampler : register(s0);
SamplerComparisonState gShadowSampler : register(s1);

PixelShaderOutput main(VertexShaderOutput input) {
   
	PixelShaderOutput output;

	float4 transformUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	float4 textureColor = gTexture.Sample(gSampler, transformUV.xy);
	
	//========================================================================*/
	//* Lighting *//
	
	if (gMaterial.enableLighting == 1) {
		
		if (gMaterial.enableHalfLambert == 1) {

			float NdotL = dot(normalize(input.normal), normalize(-gPunctual.directionalLight.direction));
			float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
			
			// rgb
			output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gPunctual.directionalLight.color.rgb * cos * gPunctual.directionalLight.intensity;
		} else {

			float cos = saturate(dot(normalize(input.normal), -gPunctual.directionalLight.direction));
			
			// rgb
			output.color.rgb =
			gMaterial.color.rgb * textureColor.rgb * gPunctual.directionalLight.color.rgb * cos * gPunctual.directionalLight.intensity;
		}
		
		if (gMaterial.enablePhongReflection == 1) {
			
			// PointLight�̓��ˌ�
			float3 pointLightDirection = normalize(input.worldPosition - gPunctual.pointLight.pos);
			// PointLight�ւ̋���
			float distancePointLight = length(gPunctual.pointLight.pos - input.worldPosition);
			// �w���ɂ��R���g���[��
			float factorPointLight = pow(saturate(-distancePointLight / gPunctual.pointLight.radius + 1.0f), gPunctual.pointLight.decay);
			// SpotLight�̓��ˌ�
			float3 spotLightDirectionOnSurface = normalize(input.worldPosition - gPunctual.spotLight.pos);
			// SpotLight�ւ̋���
			float distanceSpotLight = length(gPunctual.spotLight.pos - input.worldPosition);
			// �w���ɂ��R���g���[��
			float factorSpotLight = pow(saturate(-distanceSpotLight / gPunctual.spotLight.distance + 1.0f), gPunctual.spotLight.decay);
			// Camera�����Z�o
			float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);

			/*-------------------------------------------------------------------------------------------------*/
			/// PointLight

			// ���ˌ��̔��˃x�N�g���̌v�Z
			float3 reflectPointLight = reflect(pointLightDirection, normalize(input.normal));
			float RdotEPointLight = dot(reflectPointLight, toEye);
			float specularPowPointLight = pow(saturate(RdotEPointLight), gMaterial.shininess);
			float NdotLPointLight = dot(normalize(input.normal), -pointLightDirection);
			float cosPointLight = pow(NdotLPointLight * 0.5f + 0.5f, 2.0f);
			// �g�U����
			float3 diffusePointLight =
			gMaterial.color.rgb * textureColor.rgb * gPunctual.pointLight.color.rgb * cosPointLight * gPunctual.pointLight.intensity * factorPointLight;
			// ���ʔ���
			float3 specularPointLight =
			gPunctual.pointLight.color.rgb * gPunctual.pointLight.intensity * factorPointLight * specularPowPointLight * gMaterial.specularColor;
			
			/*-------------------------------------------------------------------------------------------------*/
			/// SpotLight
			
			// ���ˌ��̔��˃x�N�g���̌v�Z
			float3 reflectSpotLight = reflect(spotLightDirectionOnSurface, normalize(input.normal));
			float RdotESpotLight = dot(reflectSpotLight, toEye);
			float specularPowSpotLight = pow(saturate(RdotESpotLight), gMaterial.shininess);
			float NdotLSpotLight = dot(normalize(input.normal), -spotLightDirectionOnSurface);
			float cosSpotLight = pow(NdotLSpotLight * 0.5f + 0.5f, 2.0f);
			float cosAngle = dot(spotLightDirectionOnSurface, gPunctual.spotLight.direction);
			float falloffFactor = saturate((cosAngle - gPunctual.spotLight.cosAngle) / (gPunctual.spotLight.cosFalloffStart - gPunctual.spotLight.cosAngle));
			// �g�U����
			float3 diffuseSpotLight =
			gMaterial.color.rgb * textureColor.rgb * gPunctual.spotLight.color.rgb * cosSpotLight * gPunctual.spotLight.intensity * factorSpotLight * falloffFactor;
			// ���ʔ���
			float3 specularSpotLight =
			gPunctual.spotLight.color.rgb * gPunctual.spotLight.intensity * factorSpotLight * falloffFactor * specularPowSpotLight * gMaterial.specularColor;

			/*-------------------------------------------------------------------------------------------------*/

			// �g�U���� + ���ʔ��� PointLight + SpotLight
			output.color.rgb += diffusePointLight + specularPointLight + diffuseSpotLight + specularSpotLight;
		}
		
		if (gMaterial.enableBlinnPhongReflection == 1) {

			// PointLight�̓��ˌ�
			float3 pointLightDirection = normalize(input.worldPosition - gPunctual.pointLight.pos);
			// PointLight�ւ̋���
			float distancePointLight = length(gPunctual.pointLight.pos - input.worldPosition);
			// �w���ɂ��R���g���[��
			float factorPointLight = pow(saturate(-distancePointLight / gPunctual.pointLight.radius + 1.0f), gPunctual.pointLight.decay);
			// SpotLight�̓��ˌ�
			float3 spotLightDirectionOnSurface = normalize(input.worldPosition - gPunctual.spotLight.pos);
			// SpotLight�ւ̋���
			float distanceSpotLight = length(gPunctual.spotLight.pos - input.worldPosition);
			// �w���ɂ��R���g���[��
			float factorSpotLight = pow(saturate(-distanceSpotLight / gPunctual.spotLight.distance + 1.0f), gPunctual.spotLight.decay);
			// Camera�����Z�o
			float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);

			/*-------------------------------------------------------------------------------------------------*/
			/// PointLight

			float3 halfVectorPointLight = normalize(-pointLightDirection + toEye);
			float NDotHPointLight = dot(normalize(input.normal), halfVectorPointLight);
			float specularPowPointLight = pow(saturate(NDotHPointLight), gMaterial.shininess);

			float NdotLPointLight = dot(normalize(input.normal), -pointLightDirection);
			float cosPointLight = pow(NdotLPointLight * 0.5f + 0.5f, 2.0f);

			// �g�U����
			float3 diffusePointLight =
			gMaterial.color.rgb * textureColor.rgb * gPunctual.pointLight.color.rgb * cosPointLight * gPunctual.pointLight.intensity * factorPointLight;

			// ���ʔ���
			float3 specularPointLight =
			gPunctual.pointLight.color.rgb * gPunctual.pointLight.intensity * factorPointLight * specularPowPointLight * gMaterial.specularColor;

			/*-------------------------------------------------------------------------------------------------*/
			/// SpotLight

			float3 halfVectorSpotLight = normalize(-spotLightDirectionOnSurface + toEye);
			float NDotHSpotLight = dot(normalize(input.normal), halfVectorSpotLight);
			float specularPowSpotLight = pow(saturate(NDotHSpotLight), gMaterial.shininess);

			float NdotLSpotLight = dot(normalize(input.normal), -spotLightDirectionOnSurface);
			float cosSpotLight = pow(NdotLSpotLight * 0.5f + 0.5f, 2.0f);

			float cosAngle = dot(spotLightDirectionOnSurface, gPunctual.spotLight.direction);
			float falloffFactor = saturate((cosAngle - gPunctual.spotLight.cosAngle) / (gPunctual.spotLight.cosFalloffStart - gPunctual.spotLight.cosAngle));

			// �g�U����
			float3 diffuseSpotLight =
			gMaterial.color.rgb * textureColor.rgb * gPunctual.spotLight.color.rgb * cosSpotLight * gPunctual.spotLight.intensity * falloffFactor * factorSpotLight;

			// ���ʔ���
			float3 specularSpotLight =
			gPunctual.spotLight.color.rgb * gPunctual.spotLight.intensity * falloffFactor * factorSpotLight * specularPowSpotLight * gMaterial.specularColor;

			/*-------------------------------------------------------------------------------------------------*/

			// �g�U���� + ���ʔ��� LightDirecion + PointLight + SpotLight
			output.color.rgb += diffusePointLight + specularPointLight + diffuseSpotLight + specularSpotLight;
		}
	} else {

		output.color.rgb = gMaterial.color.rgb * textureColor.rgb;
	}
	
	//========================================================================*/
	//* emission *//
	
	// �����F
	float3 emissionColor = gMaterial.emissionColor * gMaterial.emissiveIntensity;
	// Emission�����Z
	output.color.rgb += emissionColor * textureColor.rgb;
	
	//========================================================================*/
	//* shadow *//
	
	// w���Z�Ő��K���X�N���[�����W�n�ɕϊ�����
	float2 shadowMapUV = input.positionInLVP.xy / input.positionInLVP.w;
	shadowMapUV *= float2(0.5f, -0.5f);
	shadowMapUV += 0.5f;
	
	// ���C�g�r���[�X�N���[����Ԃł�Z�l���v�Z����
	float zInLVp = input.positionInLVP.z / input.positionInLVP.w;
	
	if (shadowMapUV.x > 0.0f && shadowMapUV.x < 1.0f &&
		shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f) {
		
		// �@���ƌ��̕����̓��ς��v�Z
		float NdotL = max(0.0f, dot(input.normal, gPunctual.directionalLight.direction));

		float slopeScale = 1.0f;
		float constantBias = 0.001f;
		float bias = NdotL + constantBias;
		
		// �@�����������Ȃ�e�𖳌���
		if (abs(input.normal.y) < 0.1f) {
			bias = 1.0f;
		}

		// �V���h�E�}�b�v�̐[�x�l���擾
		float zInShadowMap = gShadowTexture.Sample(gSampler, shadowMapUV).r;

		if (zInLVp - bias > zInShadowMap) {
			
			// �Օ����̎擾
			float shadow = gShadowTexture.SampleCmpLevelZero(
			gShadowSampler, shadowMapUV, zInLVp);

			float3 shadowColor = output.color.rgb * gMaterial.shadowRate;
			output.color.rgb = lerp(output.color.rgb, shadowColor, shadow);
		}
	}

	// ���l
	output.color.a = gMaterial.color.a * textureColor.a;
	
	return output;
}