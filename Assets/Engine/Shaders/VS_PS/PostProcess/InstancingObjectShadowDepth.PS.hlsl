#include "ShadowDepth.hlsli"

float4 main(VertexShaderOutput input) : SV_TARGET0 {
	
	// Z�l�̏�������
	return float4(input.position.z, input.position.z, input.position.z, 1.0f);
}