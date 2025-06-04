//============================================================================
//	include
//============================================================================

#include "Object2D.hlsli"

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
	
	float4x4 uvTransform;
	float4 color;
	float3 emissionColor;
	uint useVertexColor;
	float emissiveIntensity;
};

//============================================================================
//	Texture Sampler
//============================================================================

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

//============================================================================
//	Main
//============================================================================
PSOutput main(VSOutput input) {
	
	PSOutput output;
	
	float4 transformUV = mul(float4(input.texcoord, 0.0f, 1.0f), uvTransform);
	float4 textureColor = gTexture.Sample(gSampler, transformUV.xy);
	
	if (textureColor.a <= 0.25f) {
		discard;
	}

	output.color.rgb = color.rgb * textureColor.rgb;
	// ���l
	output.color.a = color.a * input.color.a * textureColor.a;
	
	//���_�J���[�K��
	if (useVertexColor == 1) {

		// rgb�̂�
		output.color.rgb *= input.color.rgb;
	}
	
	// emission����
	// �����F
	float3 emission = emissionColor * emissiveIntensity;
	// emission�����Z
	output.color.rgb += emission * textureColor.rgb;

	return output;
}