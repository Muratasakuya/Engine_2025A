
//============================================================================*/
//	HorizonBlur.CS
//============================================================================*/

struct BlurParam {
	
	int radius;
	float sigma;
};

RWTexture2D<float4> gOutputTexture : register(u0);
Texture2D<float4> gInputTexture : register(t0);
ConstantBuffer<BlurParam> gBlurParam : register(b0);

// ガウス関数
float Gaussian(float x, float sigma) {
	
	return exp(-(x * x) / (2.0f * sigma * sigma)) / (sqrt(2.0f * 3.141592653589793) * sigma);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	float4 color = 0;
	float weightSum = 0.0;

	for (int i = -gBlurParam.radius; i <= gBlurParam.radius; i++) {

		int2 offset = int2(i, 0);
		float weight = Gaussian(float(i), gBlurParam.sigma); // ガウス分布計算

		color += gInputTexture.Load(int3(DTid.xy + offset, 0)) * weight;
		weightSum += weight;
	}

	// 正規化
	gOutputTexture[DTid.xy] = color / weightSum;
}