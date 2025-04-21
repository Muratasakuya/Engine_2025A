//============================================================================
//	include
//============================================================================

#include "EffectMSFunction.hlsli"

//============================================================================
//	Output
//============================================================================

struct PSOutput {
	
	float4 color : SV_TARGET0;
};

//============================================================================
//	Main
//============================================================================
PSOutput main(MSOutput input) {
	
	PSOutput output;

	// uv
	float4 transformUV = mul(float4(input.texcoord, 0.0f, 1.0f), uvTransform);
	// diffuseColor
	float4 diffuseColor = GetDiffuseColor(transformUV, input);
	
	// discard�ɂ��pixel���p
	if (diffuseColor.a < alphaReference) {
		discard;
	}
	
	// �F
	output.color.rgb = color.rgb * diffuseColor.rgb;
	// ���l
	output.color.a = color.a * input.vertexColor.a * diffuseColor.a;
	
	//���_�J���[�K��
	if (useVertexColor == 1) {

		// rgb�̂�
		output.color.rgb *= input.vertexColor.rgb;
	}
	
	// emission����
	// �����F
	float3 emission = emissionColor * emissiveIntensity;
	// emission�����Z
	output.color.rgb += emission * diffuseColor.rgb;
	
	return output;
}