//============================================================================
//	include
//============================================================================

#include "Skybox.hlsli"

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
};

//============================================================================
//	texture Sampler
//============================================================================

TextureCube<float4> gTextures[] : register(t0, space0);

SamplerState gSampler : register(s0);

//============================================================================
//	Main
//============================================================================
PSOutput main(VSOutput input) {

	PSOutput output;

	float4 textureColor = gTextures[textureIndex].Sample(gSampler, input.texcoord);
	output.color = textureColor * color;
	
	return output;
}