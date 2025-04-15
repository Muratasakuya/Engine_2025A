//============================================================================
//	include
//============================================================================

#include "EffectMesh.hlsli"

//============================================================================
//	Output
//============================================================================

struct PSOutput {
	
	float4 color : SV_TARGET0;
};

//============================================================================
//	CBuffer
//============================================================================

cbuffer Material : register(b0) {

	float4 color;
	uint textureIndex;
	float emissiveIntensity;
	float3 emissionColor;
	float4x4 uvTransform;
};

//============================================================================
//	Texture Sampler
//============================================================================

Texture2D<float4> gTextures[] : register(t0, space0);
SamplerState gSampler : register(s0);

//============================================================================
//	Main
//============================================================================
PSOutput main(MSOutput input) {
	
	PSOutput output;

	float4 transformUV = mul(float4(input.texcoord, 0.0f, 1.0f), uvTransform);
	float4 textureColor = gTextures[textureIndex].Sample(gSampler, transformUV.xy);
	
	// discard�A�l��CBuffer�œn����悤�ɂ���
	if (textureColor.a < 0.5f) {
		discard;
	}
	
	// �F
	output.color.rgb = color.rgb * textureColor.rgb;
	
	// emission����
	// �����F
	float3 emission = emissionColor * emissiveIntensity;
	// emission�����Z
	output.color.rgb += emission * textureColor.rgb;

	// ���l
	output.color.a = color.a * textureColor.a;
	
	return output;
}