#include "GraphicsCore.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Core/Window/WinApp.h>
#include <Engine/Renderer/LineRenderer.h>
#include <Engine/Scene/Camera/CameraManager.h>
#include <Engine/Scene/Light/LightManager.h>
#include <Engine/Core/Graphics/Skybox/Skybox.h>
#include <Engine/Particle/ParticleSystem.h>
#include <Engine/Config.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

//============================================================================
//	GraphicsCore classMethods
//============================================================================

void GraphicsCore::InitDXDevice() {
#ifdef _DEBUG
	ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {

		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();

		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	dxDevice_->Create();

#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(dxDevice_->Get()->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

		// やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {

			// https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);
	}

	// shaderModelのチェック
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
	auto hr = dxDevice_->Get()->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel));
	if (FAILED(hr) || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5)) {

		ASSERT(FALSE, "shaderModel 6.5 is not supported");
	}

	// meshShaderに対応しているかチェック
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
	hr = dxDevice_->Get()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features));
	if (FAILED(hr) || (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)) {

		ASSERT(FALSE, "meshShaders aren't supported");
	}
#endif
}

void GraphicsCore::InitRenderTexture() {

	ID3D12Device8* device = dxDevice_->Get();

	// renderTexture作成
	renderTexture_ = std::make_unique<RenderTexture>();
	renderTexture_->Create(
		Config::kWindowWidth, Config::kWindowHeight,
		Color(Config::kWindowClearColor[0], Config::kWindowClearColor[1],
			Config::kWindowClearColor[2], Config::kWindowClearColor[3]),
		Config::kRenderTextureRTVFormat, device, rtvDescriptor_.get(), srvDescriptor_.get());

	// gui用texture作成
#ifdef _DEBUG
	guiRenderTexture_ = std::make_unique<GuiRenderTexture>();
	guiRenderTexture_->Create(Config::kWindowWidth, Config::kWindowHeight, Config::kSwapChainRTVFormat,
		device, srvDescriptor_.get());
#endif // _DEBUG

	// debugSceneRenderTexture作成
#ifdef _DEBUG
	// renderTexture
	debugSceneRenderTexture_ = std::make_unique<RenderTexture>();
	debugSceneRenderTexture_->Create(Config::kWindowWidth, Config::kWindowHeight,
		Color(Config::kWindowClearColor[0], Config::kWindowClearColor[1],
			Config::kWindowClearColor[2], Config::kWindowClearColor[3]),
		Config::kRenderTextureRTVFormat, device, rtvDescriptor_.get(), srvDescriptor_.get());
#endif // _DEBUG

	// shadowMap作成
	shadowMap_ = std::make_unique<ShadowMap>();
	shadowMap_->Create(Config::kShadowMapSize, Config::kShadowMapSize,
		dsvDescriptor_.get(), srvDescriptor_.get());
}

void GraphicsCore::Init(WinApp* winApp) {

	//============================================================================
	//	init: directX
	//============================================================================

	// device初期化
	dxDevice_ = std::make_unique<DxDevice>();
	InitDXDevice();
	ID3D12Device8* device = dxDevice_->Get();

	// command初期化
	dxCommand_ = std::make_unique<DxCommand>();
	dxCommand_->Create(device);

	// DXC初期化
	dxShaderComplier_ = std::make_unique<DxShaderCompiler>();
	dxShaderComplier_->Init();

	// RTV初期化
	rtvDescriptor_ = std::make_unique<RTVDescriptor>();
	rtvDescriptor_->Init(device, DescriptorType(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

	// 画面設定
	dxSwapChain_ = std::make_unique<DxSwapChain>();
	dxSwapChain_->Create(winApp, dxDevice_->GetDxgiFactory(), dxCommand_->GetQueue(), rtvDescriptor_.get());

	// DSV初期化
	dsvDescriptor_ = std::make_unique<DSVDescriptor>();
	dsvDescriptor_->Init(device, DescriptorType(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
	dsvDescriptor_->InitFrameBufferDSV();

	// SRV初期化
	srvDescriptor_ = std::make_unique<SRVDescriptor>();
	srvDescriptor_->Init(device, DescriptorType(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));

#ifdef _DEBUG
	imguiManager_ = std::make_unique<ImGuiManager>();
	imguiManager_->Init(winApp->GetHwnd(),
		dxSwapChain_->GetDesc().BufferCount, device, srvDescriptor_.get());
#endif // _DEBUG

	// renderTexture初期化
	InitRenderTexture();

	// postProcessSystem初期化
	postProcessSystem_ = std::make_unique<PostProcessSystem>();
	postProcessSystem_->Init(device, dxShaderComplier_.get(), srvDescriptor_.get());

	// skinning用pipeline作成
	skinningPipeline_ = std::make_unique<PipelineState>();
	skinningPipeline_->Create("Skinning.json",
		dxDevice_->Get(), srvDescriptor_.get(), dxShaderComplier_.get());

	// mesh描画初期化
	meshRenderer_ = std::make_unique<MeshRenderer>();
	meshRenderer_->Init(dxDevice_->Get(), shadowMap_.get(), dxShaderComplier_.get(), srvDescriptor_.get());

	// sprite描画初期化
	spriteRenderer_ = std::make_unique<SpriteRenderer>();
	spriteRenderer_->Init(dxDevice_->Get(), srvDescriptor_.get(), dxShaderComplier_.get());

	// sceneBuffer作成
	sceneBuffer_ = std::make_unique<SceneConstBuffer>();
	sceneBuffer_->Create(device);
}

void GraphicsCore::Finalize(HWND hwnd) {

#ifdef _DEBUG
	imguiManager_->Finalize();
	imguiManager_.reset();
#endif

	dxCommand_->Finalize(hwnd);

	dxDevice_.reset();
	dxCommand_.reset();
	rtvDescriptor_.reset();
	dxSwapChain_.reset();
	dsvDescriptor_.reset();
	srvDescriptor_.reset();
	dxShaderComplier_.reset();
	renderTexture_.reset();
#ifdef _DEBUG
	debugSceneRenderTexture_.reset();
#endif // _DEBUG
	shadowMap_.reset();
	postProcessSystem_.reset();
	meshRenderer_.reset();
	spriteRenderer_.reset();

	Skybox::GetInstance()->Finalize();
}

//============================================================================
//	Begin
//============================================================================

void GraphicsCore::BeginFrame() {

#ifdef _DEBUG
	imguiManager_->Begin();
#endif

	// srvDescriptorHeap設定
	dxCommand_->SetDescriptorHeaps({ srvDescriptor_->GetDescriptorHeap() });
}

//============================================================================
//	Rendering Pass
//============================================================================

void GraphicsCore::Render(CameraManager* cameraManager,
	LightManager* lightManager) {

	// bufferの更新
	sceneBuffer_->Update(cameraManager, lightManager);
	spriteRenderer_->Update(cameraManager);

	// skybox更新
	Skybox::GetInstance()->Update();

	// zPass
	RenderZPass();
	// offscreenTexture
	RenderOffscreenTexture();
#ifdef _DEBUG
	// debugSceneRenderTexture
	RenderDebugSceneRenderTexture();
#endif // _DEBUG

	// frameBuffer
	RenderFrameBuffer();

	// commandの実行
	EndRenderFrame();
}

void GraphicsCore::DebugUpdate() {

	// skinning用のCSpipelineを設定
	ID3D12GraphicsCommandList* commandList = dxCommand_->GetCommandList();
	commandList->SetComputeRootSignature(skinningPipeline_->GetRootSignature());
	commandList->SetPipelineState(skinningPipeline_->GetComputePipeline());
}

//============================================================================
//	Main
//============================================================================

void GraphicsCore::RenderZPass() {

	// srvDescriptorHeap設定
	dxCommand_->SetDescriptorHeaps({ srvDescriptor_->GetDescriptorHeap() });

	dxCommand_->SetRenderTargets(std::nullopt, shadowMap_->GetCPUHandle());
	dxCommand_->ClearDepthStencilView(shadowMap_->GetCPUHandle());
	dxCommand_->SetViewportAndScissor(Config::kShadowMapSize, Config::kShadowMapSize);

	// Z値描画
	meshRenderer_->RenderingZPass(sceneBuffer_.get(), dxCommand_.get());

	// Write -> PixelShader
	dxCommand_->TransitionBarriers({ shadowMap_->GetResource() },
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void GraphicsCore::RenderOffscreenTexture() {

	dxCommand_->SetRenderTargets(renderTexture_->GetRenderTarget(),
		dsvDescriptor_->GetFrameCPUHandle());
	dxCommand_->ClearDepthStencilView(dsvDescriptor_->GetFrameCPUHandle());
	dxCommand_->SetViewportAndScissor(Config::kWindowWidth, Config::kWindowHeight);

	// 描画処理
	Renderers(false);

	// RenderTarget -> ComputeShader
	dxCommand_->TransitionBarriers({ renderTexture_->GetResource() },
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// postProcess処理実行
	postProcessSystem_->Execute(renderTexture_.get(), dxCommand_.get());
}

void GraphicsCore::RenderDebugSceneRenderTexture() {

	dxCommand_->SetRenderTargets(debugSceneRenderTexture_->GetRenderTarget(),
		dsvDescriptor_->GetFrameCPUHandle());
	dxCommand_->ClearDepthStencilView(dsvDescriptor_->GetFrameCPUHandle());
	dxCommand_->SetViewportAndScissor(Config::kWindowWidth, Config::kWindowHeight);

	// 描画処理
	Renderers(true);

	// RenderTarget -> ComputeShader
	dxCommand_->TransitionBarriers({ debugSceneRenderTexture_->GetResource() },
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// bloom処理を行う
	postProcessSystem_->ExecuteDebugScene(debugSceneRenderTexture_.get(), dxCommand_.get());
}

void GraphicsCore::RenderFrameBuffer() {

	// RenderTarget -> Present
	dxCommand_->TransitionBarriers({ dxSwapChain_->GetCurrentResource() },
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	dxCommand_->SetRenderTargets(dxSwapChain_->GetRenderTarget(),
		dsvDescriptor_->GetFrameCPUHandle());
	dxCommand_->SetViewportAndScissor(Config::kWindowWidth, Config::kWindowHeight);

	// frameBufferへ結果を描画
	postProcessSystem_->RenderFrameBuffer(dxCommand_.get());

	// sprite描画、postPrecessを適用しない
	//spriteRenderer_->RenderIrrelevant(dxCommand_->GetCommandList());
}

void GraphicsCore::Renderers(bool debugEnable) {

	// sprite描画、postPrecess適用
	// model描画前
	//spriteRenderer_->RenderApply(SpriteLayer::PreModel, dxCommand_->GetCommandList());

	// line描画実行
	LineRenderer::GetInstance()->ExecuteLine(debugEnable);

	// 通常描画処理
	meshRenderer_->Rendering(debugEnable, sceneBuffer_.get(), dxCommand_.get());

	// particle描画
	ParticleSystem::GetInstance()->Rendering(debugEnable,
		sceneBuffer_.get(), dxCommand_->GetCommandList());

	// sprite描画、postPrecess適用
	// model描画後
	//spriteRenderer_->RenderApply(SpriteLayer::PostModel, dxCommand_->GetCommandList());
}

//============================================================================
//	End
//============================================================================

void GraphicsCore::EndRenderFrame() {

#ifdef _DEBUG

	// gui描画用のtextureをframeBufferからコピー
	dxCommand_->CopyTexture(
		guiRenderTexture_->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		dxSwapChain_->GetCurrentResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// imgui描画
	imguiManager_->End();
	imguiManager_->Draw(dxCommand_->GetCommandList());

	// ComputeShader -> RenderTarget
	dxCommand_->TransitionBarriers({ debugSceneRenderTexture_->GetResource() },
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
#endif // _DEBUG

	// PixelShader -> Write
	dxCommand_->TransitionBarriers({ shadowMap_->GetResource() },
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	// ComputeShader -> RenderTarget
	dxCommand_->TransitionBarriers({ renderTexture_->GetResource() },
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	// Present -> RenderTarget
	dxCommand_->TransitionBarriers({ dxSwapChain_->GetCurrentResource() },
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	// CSへの書き込み状態へ遷移
	postProcessSystem_->ToWrite(dxCommand_.get());

	// Command実行
	dxCommand_->ExecuteCommands(dxSwapChain_->Get());
}