//============================================================================
//	include
//============================================================================

#include "Triangle.hlsli"

//============================================================================
//	Input
//============================================================================

struct MSInput {
	
	float4 position;
	float4 color;
};

//============================================================================
//	CBuffer structure
//============================================================================

// Transform
struct Transform {
	
	float4x4 world;
	float4x4 viewProjection;
};

//============================================================================
//	Resources
//============================================================================

StructuredBuffer<MSInput> gVertices : register(t0);
StructuredBuffer<uint3> gIndices : register(t1);
ConstantBuffer<Transform> gTransform : register(b0);

//============================================================================
//	Main
//============================================================================

[numthreads(64, 1, 1)]
[outputtopology("triangle")]
void main(uint groupIndex : SV_GroupIndex,
out vertices MSOutput verts[3], out indices uint3 tris[1]) {
	
	// �X���b�h�O���[�v�̒��_�ƁA�v���~�e�B�u�̐���ݒ�
	SetMeshOutputCounts(3, 1);
	
	// ���_�ԍ���ݒ�
	if (groupIndex < 1) {
		
		tris[groupIndex] = gIndices[groupIndex];
	}
	
	// ���_�f�[�^��ݒ�
	if (groupIndex < 3) {
		
		MSOutput output = (MSOutput) 0;

		float4 worldPos = mul(gTransform.world, gVertices[groupIndex].position);
		float4 projPos = mul(gTransform.viewProjection, worldPos);

		output.position = projPos;
		output.color = gVertices[groupIndex].color;

		verts[groupIndex] = output;
	}
}