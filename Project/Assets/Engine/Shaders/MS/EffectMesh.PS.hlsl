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
	
	// instanceId、Pixelごとの処理
	uint id = input.instanceID;
	Material material = gMaterials[id];
	
	// uv
	float4 transformUV = mul(float4(input.texcoord, 0.0f, 1.0f), material.uvTransform);
	
	// noiceTextureによるpixel棄却、edge判定
	float4 edgeColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		
	if (material.useNoiseTexture == 1) {
		if (ApplyNoiseDiscardAndEdge(id, transformUV, edgeColor)) {
		
			output.color = edgeColor;
			// emission処理
			// 発光色
			float3 emission = material.edgeEmissionColor * material.edgeEmissiveIntensity;
			// emissionを加算
			output.color.rgb += emission;
		
			return output;
		}
	}
	
	// textureColorの取得、alpha値のpixel棄却判定
	float4 textureColor = GetTextureColor(id, input, transformUV);
	if (textureColor.a < material.textureAlphaReference) {
		discard;
	}
	
	// 色
	output.color.rgb = material.color.rgb * textureColor.rgb;
	
	// emission処理
	// 発光色
	float3 emission = material.emissionColor * material.emissiveIntensity;
	// emissionを加算
	output.color.rgb += emission * textureColor.rgb;
	
	//頂点カラー適応
	if (material.useVertexColor == 1) {

		// rgbのみ
		output.color.rgb *= input.vertexColor.rgb;
	}
	
	// α値
	output.color.a = material.color.a * input.vertexColor.a * textureColor.a;
	
	return output;
}