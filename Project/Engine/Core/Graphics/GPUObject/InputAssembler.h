#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetStructure.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>

//============================================================================
//	InputAssembler class
//============================================================================
class InputAssembler {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	InputAssembler() = default;
	~InputAssembler() = default;

	void Init(const MeshModelData& meshData, ID3D12Device* device);

	//--------- accessor -----------------------------------------------------

	const DxConstBuffer<MeshVertex>& GetVertexBuffer(uint32_t meshIndex) const { return vertices_[meshIndex]; }
	const DxConstBuffer<uint32_t>& GetIndexBuffer(uint32_t meshIndex) const { return indices_[meshIndex]; }

	UINT GetVertexCount(uint32_t meshIndex) const { return vertexCounts_[meshIndex]; }
	UINT GetIndexCount(uint32_t meshIndex) const { return indexCounts_[meshIndex]; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::vector<DxConstBuffer<MeshVertex>> vertices_;

	std::vector<DxConstBuffer<uint32_t>> indices_;

	std::vector<UINT> vertexCounts_;
	std::vector<UINT> indexCounts_;
};