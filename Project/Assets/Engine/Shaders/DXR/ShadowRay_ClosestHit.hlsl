//============================================================================
//	include
//============================================================================

#include "ShadowRay.hlsli"

//============================================================================
//	Closesthit
//============================================================================
[shader("closesthit")]
void ShadowClosestHit(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attributes) {
	
	// �Փ˂���
	payload.isHit = 1;
}