//============================================================================
//	include
//============================================================================

#include "Basic.hlsli"

//============================================================================
//	Output
//============================================================================

struct PSOutput {
	
	float4 color : SV_TARGET0;
};

//============================================================================
//	CBuffer
//============================================================================

cbuffer PunctualLight : register(b1) {
	
	DirectionalLight directionalLight;
	PointLight pointLight;
	SpotLight spotLight;
};

cbuffer Camera : register(b2) {

	float3 worldPosition;
};

//============================================================================
//	StructuredBuffer
//============================================================================

struct Material {

	float4 color;
	uint textureIndex;
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

StructuredBuffer<Material> gMaterials : register(t0);

//============================================================================
//	Texture Sampler
//============================================================================

Texture2D<float4> gTextures[] : register(t1, space0);
Texture2D<float3> gShadowTexture : register(t2, space1);

SamplerState gSampler : register(s0);
SamplerComparisonState gShadowSampler : register(s1);

//============================================================================
//	Main
//============================================================================
PSOutput main(MSOutput input) {
	
	PSOutput output;
	
	// instanceId�APixel���Ƃ̏���
	uint id = input.instanceID;

	float4 transformUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterials[id].uvTransform);
	float4 textureColor = gTextures[gMaterials[id].textureIndex].Sample(gSampler, transformUV.xy);
	
	//========================================================================*/
	//* Lighting *//
	
	if (gMaterials[id].enableLighting == 1) {
		
		if (gMaterials[id].enableHalfLambert == 1) {

			float NdotL = dot(normalize(input.normal), normalize(-directionalLight.direction));
			float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
			
			// rgb
			output.color.rgb = gMaterials[id].color.rgb * textureColor.rgb * directionalLight.color.rgb * cos * directionalLight.intensity;
		} else {

			float cos = saturate(dot(normalize(input.normal), -directionalLight.direction));
			
			// rgb
			output.color.rgb =
			gMaterials[id].color.rgb * textureColor.rgb * directionalLight.color.rgb * cos * directionalLight.intensity;
		}
		
		if (gMaterials[id].enablePhongReflection == 1) {
			
			// PointLight�̓��ˌ�
			float3 pointLightDirection = normalize(input.worldPosition - pointLight.pos);
			// PointLight�ւ̋���
			float distancePointLight = length(pointLight.pos - input.worldPosition);
			// �w���ɂ��R���g���[��
			float factorPointLight = pow(saturate(-distancePointLight / pointLight.radius + 1.0f), pointLight.decay);
			// SpotLight�̓��ˌ�
			float3 spotLightDirectionOnSurface = normalize(input.worldPosition - spotLight.pos);
			// SpotLight�ւ̋���
			float distanceSpotLight = length(spotLight.pos - input.worldPosition);
			// �w���ɂ��R���g���[��
			float factorSpotLight = pow(saturate(-distanceSpotLight / spotLight.distance + 1.0f), spotLight.decay);
			// Camera�����Z�o
			float3 toEye = normalize(worldPosition - input.worldPosition);

			/*-------------------------------------------------------------------------------------------------*/
			/// PointLight

			// ���ˌ��̔��˃x�N�g���̌v�Z
			float3 reflectPointLight = reflect(pointLightDirection, normalize(input.normal));
			float RdotEPointLight = dot(reflectPointLight, toEye);
			float specularPowPointLight = pow(saturate(RdotEPointLight), gMaterials[id].shininess);
			float NdotLPointLight = dot(normalize(input.normal), -pointLightDirection);
			float cosPointLight = pow(NdotLPointLight * 0.5f + 0.5f, 2.0f);
			// �g�U����
			float3 diffusePointLight =
			gMaterials[id].color.rgb * textureColor.rgb * pointLight.color.rgb * cosPointLight * pointLight.intensity * factorPointLight;
			// ���ʔ���
			float3 specularPointLight =
			pointLight.color.rgb * pointLight.intensity * factorPointLight * specularPowPointLight * gMaterials[id].specularColor;
			
			/*-------------------------------------------------------------------------------------------------*/
			/// SpotLight
			
			// ���ˌ��̔��˃x�N�g���̌v�Z
			float3 reflectSpotLight = reflect(spotLightDirectionOnSurface, normalize(input.normal));
			float RdotESpotLight = dot(reflectSpotLight, toEye);
			float specularPowSpotLight = pow(saturate(RdotESpotLight), gMaterials[id].shininess);
			float NdotLSpotLight = dot(normalize(input.normal), -spotLightDirectionOnSurface);
			float cosSpotLight = pow(NdotLSpotLight * 0.5f + 0.5f, 2.0f);
			float cosAngle = dot(spotLightDirectionOnSurface, spotLight.direction);
			float falloffFactor = saturate((cosAngle - spotLight.cosAngle) / (spotLight.cosFalloffStart - spotLight.cosAngle));
			// �g�U����
			float3 diffuseSpotLight =
			gMaterials[id].color.rgb * textureColor.rgb * spotLight.color.rgb * cosSpotLight * spotLight.intensity * factorSpotLight * falloffFactor;
			// ���ʔ���
			float3 specularSpotLight =
			spotLight.color.rgb * spotLight.intensity * factorSpotLight * falloffFactor * specularPowSpotLight * gMaterials[id].specularColor;

			/*-------------------------------------------------------------------------------------------------*/

			// �g�U���� + ���ʔ��� PointLight + SpotLight
			output.color.rgb += diffusePointLight + specularPointLight + diffuseSpotLight + specularSpotLight;
		}
		
		if (gMaterials[id].enableBlinnPhongReflection == 1) {

			// PointLight�̓��ˌ�
			float3 pointLightDirection = normalize(input.worldPosition - pointLight.pos);
			// PointLight�ւ̋���
			float distancePointLight = length(pointLight.pos - input.worldPosition);
			// �w���ɂ��R���g���[��
			float factorPointLight = pow(saturate(-distancePointLight / pointLight.radius + 1.0f), pointLight.decay);
			// SpotLight�̓��ˌ�
			float3 spotLightDirectionOnSurface = normalize(input.worldPosition - spotLight.pos);
			// SpotLight�ւ̋���
			float distanceSpotLight = length(spotLight.pos - input.worldPosition);
			// �w���ɂ��R���g���[��
			float factorSpotLight = pow(saturate(-distanceSpotLight / spotLight.distance + 1.0f), spotLight.decay);
			// Camera�����Z�o
			float3 toEye = normalize(worldPosition - input.worldPosition);

			/*-------------------------------------------------------------------------------------------------*/
			/// PointLight

			float3 halfVectorPointLight = normalize(-pointLightDirection + toEye);
			float NDotHPointLight = dot(normalize(input.normal), halfVectorPointLight);
			float specularPowPointLight = pow(saturate(NDotHPointLight), gMaterials[id].shininess);

			float NdotLPointLight = dot(normalize(input.normal), -pointLightDirection);
			float cosPointLight = pow(NdotLPointLight * 0.5f + 0.5f, 2.0f);

			// �g�U����
			float3 diffusePointLight =
			gMaterials[id].color.rgb * textureColor.rgb * pointLight.color.rgb * cosPointLight * pointLight.intensity * factorPointLight;

			// ���ʔ���
			float3 specularPointLight =
			pointLight.color.rgb * pointLight.intensity * factorPointLight * specularPowPointLight * gMaterials[id].specularColor;

			/*-------------------------------------------------------------------------------------------------*/
			/// SpotLight

			float3 halfVectorSpotLight = normalize(-spotLightDirectionOnSurface + toEye);
			float NDotHSpotLight = dot(normalize(input.normal), halfVectorSpotLight);
			float specularPowSpotLight = pow(saturate(NDotHSpotLight), gMaterials[id].shininess);

			float NdotLSpotLight = dot(normalize(input.normal), -spotLightDirectionOnSurface);
			float cosSpotLight = pow(NdotLSpotLight * 0.5f + 0.5f, 2.0f);

			float cosAngle = dot(spotLightDirectionOnSurface, spotLight.direction);
			float falloffFactor = saturate((cosAngle - spotLight.cosAngle) / (spotLight.cosFalloffStart - spotLight.cosAngle));

			// �g�U����
			float3 diffuseSpotLight =
			gMaterials[id].color.rgb * textureColor.rgb * spotLight.color.rgb * cosSpotLight * spotLight.intensity * falloffFactor * factorSpotLight;

			// ���ʔ���
			float3 specularSpotLight =
			spotLight.color.rgb * spotLight.intensity * falloffFactor * factorSpotLight * specularPowSpotLight * gMaterials[id].specularColor;

			/*-------------------------------------------------------------------------------------------------*/

			// �g�U���� + ���ʔ��� LightDirecion + PointLight + SpotLight
			output.color.rgb += diffusePointLight + specularPointLight + diffuseSpotLight + specularSpotLight;
		}
	} else {

		output.color.rgb = gMaterials[id].color.rgb * textureColor.rgb;
	}
	
	//========================================================================*/
	//* emission *//
	
	// �����F
	float3 emission = gMaterials[id].emissionColor * gMaterials[id].emissiveIntensity;
	// Emission�����Z
	output.color.rgb += emission * textureColor.rgb;
	
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
		float NdotL = max(0.0f, dot(input.normal, directionalLight.direction));

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

			float3 shadowColor = output.color.rgb * gMaterials[id].shadowRate;
			output.color.rgb = lerp(output.color.rgb, shadowColor, shadow);
		}
	}

	// ���l
	output.color.a = gMaterials[id].color.a * textureColor.a;
	
	return output;
}