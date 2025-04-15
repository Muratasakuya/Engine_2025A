//============================================================================
//	include
//============================================================================

#include "EffectMesh.hlsli"

//============================================================================
//	Input
//============================================================================

struct MSInput {
	
	float4 position;
	float2 texcoord;
	float3 normal;
};

//============================================================================
//	CBuffer
//============================================================================

cbuffer TransformationMatrix : register(b0) {
	
	float4x4 world;
	float4x4 worldInverseTranspose;
};

cbuffer CameraData : register(b1) {
	
	float4x4 viewProjection;
};

//============================================================================
//	StructuredBuffer
//============================================================================

struct Meshlet {
	
	uint vertexOffset;
	uint vertexCount;
	uint primitiveOffset;
	uint primitiveCount;
	float4 color;
};

StructuredBuffer<MSInput> gVertices : register(t0);
StructuredBuffer<uint> gIndices : register(t1);
StructuredBuffer<Meshlet> gMeshlets : register(t2);
StructuredBuffer<uint> gPrimitives : register(t3);

//============================================================================
//	Function
//============================================================================

// 10bit * 3
uint3 UnpackPrimitiveIndex(uint packedIndex) {
	
	 // ����10bit�A1�ڂ̒��_�C���f�b�N�X
	 // ����10bit�A2�ڂ̒��_�C���f�b�N�X
	 // ���10bit�A3�ڂ̒��_�C���f�b�N�X
	
	return uint3(
	packedIndex & 0x3FF,
	(packedIndex >> 10) & 0x3FF,
	(packedIndex >> 20) & 0x3FF);
}

//============================================================================
//	Main
//============================================================================
[numthreads(128, 1, 1)]
[outputtopology("triangle")]
void main(
uint groupThreadId : SV_GroupThreadID, // �O���[�v���̃X���b�hID
uint groupId : SV_GroupID, // �O���[�vID
out vertices MSOutput verts[64], // �o�͒��_
out indices uint3 polys[126] // �o�͎O�p�`�C���f�b�N�X
) {
	
	// ���݂̃O���[�v�ɑΉ�����meshlet���擾
	Meshlet meshlet = gMeshlets[groupId];
	
	// ���b�V���V�F�[�_�[�̏o�͐��A���_���A�v���~�e�B�u����ݒ�
	SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);
	
	// �e�X���b�h���v���~�e�B�u�C���f�b�N�X���A���p�b�N���ďo�͔z��Ɋi�[
	if (groupThreadId < meshlet.primitiveCount) {
		
		uint packedIndex = gPrimitives[meshlet.primitiveOffset + groupThreadId];
		polys[groupThreadId] = UnpackPrimitiveIndex(packedIndex);
	}
	
	// �e�X���b�h�Œ��_�f�[�^���������ďo��
	if (groupThreadId < meshlet.vertexCount) {
		
		uint index = gIndices[meshlet.vertexOffset + groupThreadId];
		MSInput input = gVertices[index];
		MSOutput output = (MSOutput) 0;
		
		// �X�N���[�����W�ɕϊ�
		float4x4 wvp = mul(world, viewProjection);
		output.position = mul(input.position, wvp);
		
		// �e�N�X�`�����W
		output.texcoord = input.texcoord;
		
		// �@��
		output.normal = normalize(mul(input.normal, (float3x3) worldInverseTranspose));
		
		// meshlet�̐F
		output.color = meshlet.color;

		verts[groupThreadId] = output;
	}
}