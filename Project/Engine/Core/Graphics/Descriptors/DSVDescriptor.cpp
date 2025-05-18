#include "DSVDescriptor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Lib/DxUtils.h>
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Config.h>

//============================================================================
//	DSVDescriptor classMethods
//============================================================================

void DSVDescriptor::CreateDepthResource(ComPtr<ID3D12Resource>& resource,
	uint32_t width, uint32_t height,
	DXGI_FORMAT resourceFormat, DXGI_FORMAT depthClearFormat) {

	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;                                   // Textureの幅
	resourceDesc.Height = height;                                 // Textureの高さ
	resourceDesc.MipLevels = 1;                                   // mipmapの数
	resourceDesc.DepthOrArraySize = 1;                            // 奥行　or 配列Textureの配列数
	resourceDesc.Format = resourceFormat;                         // DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f(最大値)でクリア
	depthClearValue.Format = depthClearFormat; // フォーマット、Resourceに合わせる

	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties,                  // Heapの設定
		D3D12_HEAP_FLAG_NONE,             // Heapの特殊な設定、特になし
		&resourceDesc,                    // Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込む状態にしておく
		&depthClearValue,                 // Clear最適値
		IID_PPV_ARGS(&resource)           // 作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));
}

void DSVDescriptor::InitFrameBufferDSV() {

	// frameBuffer
	CreateDSV(Config::kWindowWidth, Config::kWindowHeight, dsvCPUHandle_, resource_,
		DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void DSVDescriptor::CreateDSV(uint32_t width, uint32_t height,
	D3D12_CPU_DESCRIPTOR_HANDLE& handle, ComPtr<ID3D12Resource>& resource,
	DXGI_FORMAT resourceFormat, DXGI_FORMAT depthClearFormat) {

	uint32_t index = Allocate();
	// 参照で飛び出し側も設定する
	handle = GetCPUHandle(index);

	CreateDepthResource(resource, width, height,
		resourceFormat, depthClearFormat);

	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = depthClearFormat;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	device_->CreateDepthStencilView(resource.Get(), &dsvDesc, handle);
}