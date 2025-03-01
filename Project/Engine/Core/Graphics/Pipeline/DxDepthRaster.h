#pragma once

//============================================================================
//	include
//============================================================================

// directX
#include <d3d12.h>
// json
#include <Externals/nlohmann/json.hpp>
// using
using Json = nlohmann::json;

//============================================================================
//	DxDepthRaster class
//============================================================================
class DxDepthRaster {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxDepthRaster() = default;
	~DxDepthRaster() = default;

	void Create(const Json& json, D3D12_RASTERIZER_DESC& rasterizerDesc,
		D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);
};