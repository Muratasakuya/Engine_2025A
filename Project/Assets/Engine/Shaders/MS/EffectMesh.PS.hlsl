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
	
	// instanceId�APixel���Ƃ̏���
	uint id = input.instanceID;
	Material material = gMaterials[id];
	
	// noiceTexture�ɂ��pixel���p����
	uint noiseTextureDiscardEnable = IsNoiseTextureDiscard(id, input);
	if (noiseTextureDiscardEnable == 1) {
		discard;
	}
	
	// textureColor�̎擾�Aalpha�l��pixel���p����
	float4 textureColor = GetTextureColor(id, input);
	if (textureColor.a < material.textureAlphaReference) {
		discard;
	}
	
	// �F
	output.color.rgb = material.color.rgb * textureColor.rgb;
	
	// emission����
	// �����F
	float3 emission = material.emissionColor * material.emissiveIntensity;
	// emission�����Z
	output.color.rgb += emission * textureColor.rgb;
	
	//���_�J���[�K��
	if (material.useVertexColor == 1) {

		// rgb�̂�
		output.color.rgb *= input.vertexColor.rgb;
	}
	
	// ���l
	output.color.a = material.color.a * input.vertexColor.a * textureColor.a;
	
	return output;
}