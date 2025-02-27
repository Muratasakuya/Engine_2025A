
//============================================================================*/
//	RadialBlur.CS
//============================================================================*/

struct BlurParameter {
	
	float2 center; // �u���[�̒��S
	int numSamples; // �T���v�����O��
	float width; // �ڂ����̕�
};

RWTexture2D<float4> gOutputTexture : register(u0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<BlurParameter> gBlur : register(b0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint width, height;
	gTexture.GetDimensions(width, height);

	// �s�N�Z���ʒu
	uint2 pixelPos = DTid.xy;

	// �͈͊O
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}

	// UV���W�v�Z
	float2 uv = (float2(pixelPos) + 0.5f) / float2(width, height);

	// ���˃u���[����
	float2 direction = uv - gBlur.center;

	// �u���[�K�p
	float3 outputColor = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < gBlur.numSamples; ++i) {
		
		float2 sampleUV = uv + direction * gBlur.width * float(i);
		sampleUV = clamp(sampleUV, float2(0.0f, 0.0f), float2(0.9f, 0.9f));

		outputColor += gTexture.SampleLevel(gSampler, sampleUV, 0).rgb;
	}

	// ���ω�
	outputColor *= rcp(float(gBlur.numSamples));

	gOutputTexture[pixelPos] = float4(outputColor, 1.0f);
}