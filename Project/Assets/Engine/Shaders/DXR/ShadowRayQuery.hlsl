
//============================================================================
//	ShadowRayQuery
//============================================================================

//============================================================================
//	CBuffer
//============================================================================

cbuffer Scene : register(b0) {
	
	// window
	uint2 dimensions;
	
	// light
	float3 lightDirection;
	float shadowBias;
	float shadowFar;
	
	// camera
	float4x4 inverseView;
	float4x4 inverseProjection;
};

//============================================================================
//	Buffer
//============================================================================

RaytracingAccelerationStructure gScene : register(t0);
Texture2D<float> gDepthTexture : register(t1);
RWTexture2D<float> gShadowMask : register(u0);

//============================================================================
//	function
//============================================================================

bool IsShadowed(float3 origin, float3 direction, float tMin, float tMax) {
	
	// rayDesc��ݒ�
	RayDesc ray;
	ray.Origin = origin;
	ray.Direction = direction;
	ray.TMin = tMin;
	ray.TMax = tMax;
	
	// RayQueryObject���쐬���A���s����
	RayQuery <
		RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH |
		RAY_FLAG_CULL_NON_OPAQUE |
		RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES > rayQuery;
	
	rayQuery.TraceRayInline(gScene, 0, 0xFF, ray);
	rayQuery.Proceed();
	
	return rayQuery.CommittedStatus() != COMMITTED_NOTHING;
}

//============================================================================
//	main
//============================================================================
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {

	uint2 pixel = DTid.xy;
	if (pixel.x >= dimensions.x || pixel.y >= dimensions.y) {
		return;
	}
	
	// ��ʍ��W �� NDC �� view �� world�ɕϊ�
	float2 uv = (pixel + 0.5f) / dimensions;
	float2 ndc = uv * 2.0f - 1.0f;

	float depth = gDepthTexture.Load(int3(pixel, 0));
	float4 clip = float4(ndc, depth, 1.0f);
	float4 view = mul(inverseProjection, clip);
	view /= view.w;
	
	// ���[���h���W���擾
	float3 worldPos = mul(inverseView, view).xyz;
	// �����̋t��H��
	float3 reverseLightDirection = -lightDirection;
	// �e������s��
	// true:  �e
	// false: �e����Ȃ�
	bool occluded = IsShadowed(worldPos + reverseLightDirection * shadowBias,
	reverseLightDirection, 0.0f, shadowFar);
	
	// ���茋�ʂɉ����Č��ʂ���������
	if (occluded == 1) {
		
		gShadowMask[pixel] = 0.0f;
	} else if (occluded == 0) {
		
		gShadowMask[pixel] = 1.0f;
	}
}