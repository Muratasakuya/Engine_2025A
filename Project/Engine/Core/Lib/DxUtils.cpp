#include "DxUtils.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>

//============================================================================
//	DxUtils namespaceMethods
//============================================================================

void DxUtils::MakeDescriptorHeap(ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
	ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc) {

	HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
}

void DxUtils::CreateBufferResource(ID3D12Device* device, ComPtr<ID3D12Resource>& resource, size_t sizeInBytes) {

	HRESULT hr;

	// リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	// リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// リソースのサイズ
	resourceDesc.Width = sizeInBytes;
	// バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// リソース作成
	hr = device->CreateCommittedResource(
		&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
}

bool DxUtils::CanAllocateIndex(uint32_t useIndex, uint32_t kMaxCount) {
	// FALSE: ASSERT
	return useIndex < kMaxCount;
}