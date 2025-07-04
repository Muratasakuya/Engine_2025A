#include "MeshRenderer.h"

//============================================================================
//	include
//============================================================================
// Graphics
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>
#include <Engine/Core/Graphics/PostProcess/RenderTexture.h>
#include <Engine/Core/Graphics/PostProcess/DepthTexture.h>
#include <Engine/Core/Graphics/GPUObject/SceneConstBuffer.h>
#include <Engine/Core/Graphics/Context/MeshCommandContext.h>
#include <Engine/Core/Graphics/Lib/DxUtils.h>
#include <Engine/Config.h>

// ECS
#include <Engine/Core/ECS/Core/ECSManager.h>
#include <Engine/Core/ECS/System/Systems/InstancedMeshSystem.h>
#include <Engine/Core/ECS/System/Systems/SkyboxRenderSystem.h>

//============================================================================
//	MeshRenderer classMethods
//============================================================================

void MeshRenderer::Init(ID3D12Device8* device, DxShaderCompiler* shaderCompiler, SRVDescriptor* srvDescriptor) {

	srvDescriptor_ = nullptr;
	srvDescriptor_ = srvDescriptor;

	meshShaderPipeline_ = std::make_unique<PipelineState>();
	meshShaderPipeline_->Create("MeshStandard.json", device, srvDescriptor, shaderCompiler);

	// skybox用pipeline作成
	skyboxPipeline_ = std::make_unique<PipelineState>();
	skyboxPipeline_->Create("Skybox.json", device, srvDescriptor, shaderCompiler);

	// rayScene初期化
	rayScene_ = std::make_unique<RaytracingScene>();
	rayScene_->Init(device);
}

void MeshRenderer::UpdateRayScene(DxCommand* dxCommand) {

	// commandList取得
	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();

	// 描画情報取得
	const auto& ecsSystem = ECSManager::GetInstance()->GetSystem<InstancedMeshSystem>();

	const auto& meshes = ecsSystem->GetMeshes();
	auto instancingBuffers = ecsSystem->GetInstancingData();

	if (meshes.empty()) {
		return;
	}

	// TLAS更新処理
	std::vector<IMesh*> meshPtrs;
	meshPtrs.reserve(meshes.size());
	for (auto& [_, mesh] : meshes) {

		meshPtrs.emplace_back(mesh.get());

		// BLASに渡す前に頂点を遷移
		if (mesh->IsSkinned()) {
			for (uint32_t meshIndex = 0; meshIndex < mesh->GetMeshCount(); ++meshIndex) {

				dxCommand->TransitionBarriers(
					{ static_cast<SkinnedMesh*>(mesh.get())->GetOutputVertexBuffer(meshIndex).GetResource() },
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
				);
			}
		}
	}

	// BLAS更新
	rayScene_->BuildBLASes(commandList, meshPtrs);
	std::vector<RayTracingInstance> rtInstances = ecsSystem->CollectRTInstances(rayScene_.get());
	// TLAS更新
	rayScene_->BuildTLAS(commandList, rtInstances);
}

void MeshRenderer::Rendering(bool debugEnable, SceneConstBuffer* sceneBuffer, DxCommand* dxCommand) {

	// commandList取得
	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();

	const auto& skyBoxSystem = ECSManager::GetInstance()->GetSystem<SkyboxRenderSystem>();

	// 作成されていなかったら早期リターン
	if (skyBoxSystem->IsCreated()) {

		// skybox描画
		// pipeline設定
		commandList->SetGraphicsRootSignature(skyboxPipeline_->GetRootSignature());
		commandList->SetPipelineState(skyboxPipeline_->GetGraphicsPipeline());

		// viewPro
		sceneBuffer->SetViewProCommand(debugEnable, commandList, 1);
		// texture
		commandList->SetGraphicsRootDescriptorTable(2,
			srvDescriptor_->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
		skyBoxSystem->Render(commandList);
	}

	// 描画情報取得
	const auto& ecsSystem = ECSManager::GetInstance()->GetSystem<InstancedMeshSystem>();
	MeshCommandContext commandContext{};

	const auto& meshes = ecsSystem->GetMeshes();
	auto instancingBuffers = ecsSystem->GetInstancingData();

	if (meshes.empty()) {
		return;
	}

	// renderTextureへの描画処理
	// pipeline設定
	commandList->SetGraphicsRootSignature(meshShaderPipeline_->GetRootSignature());
	commandList->SetPipelineState(meshShaderPipeline_->GetGraphicsPipeline());

	// 共通のbuffer設定
	sceneBuffer->SetMainPassCommands(debugEnable, commandList);
	// allTexture
	commandList->SetGraphicsRootDescriptorTable(11, srvDescriptor_->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

	// RayQuery
	// TLAS
	commandList->SetGraphicsRootShaderResourceView(8, rayScene_->GetTLASResource()->GetGPUVirtualAddress());
	// scene情報
	sceneBuffer->SetRaySceneCommand(commandList, 15);

	// skyboxがあるときのみ、とりあえず今は
	if (skyBoxSystem->IsCreated()) {

		// environmentTexture
		commandList->SetGraphicsRootDescriptorTable(12,
			srvDescriptor_->GetGPUHandle(skyBoxSystem->GetTextureIndex()));
	}

	for (const auto& [name, mesh] : meshes) {

		// meshごとのmatrix設定
		commandList->SetGraphicsRootShaderResourceView(4,
			instancingBuffers[name].matrixBuffer.GetResource()->GetGPUVirtualAddress());

		for (uint32_t meshIndex = 0; meshIndex < mesh->GetMeshCount(); ++meshIndex) {

			// meshごとのmaterial、lighting設定
			commandList->SetGraphicsRootShaderResourceView(9,
				instancingBuffers[name].materialsBuffer[meshIndex].GetResource()->GetGPUVirtualAddress());
			commandList->SetGraphicsRootShaderResourceView(10,
				instancingBuffers[name].lightingBuffer[meshIndex].GetResource()->GetGPUVirtualAddress());

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
			if (!debugEnable) {

				// skinnedMeshなら頂点を読める状態にする
				if (mesh->IsSkinned()) {

					dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh.get())->GetOutputVertexBuffer(meshIndex).GetResource() },
						D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
						D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
				}
			}
#else
			// skinnedMeshなら頂点を読める状態にする
			if (mesh->IsSkinned()) {

				dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh.get())->GetOutputVertexBuffer(meshIndex).GetResource() },
					D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
					D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			}
#endif

			// 描画処理
			commandContext.DispatchMesh(commandList,
				instancingBuffers[name].numInstance, meshIndex, mesh.get());

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
			if (debugEnable) {

				// skinnedMeshなら頂点を書き込み状態に戻す
				if (mesh->IsSkinned()) {

					dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh.get())->GetOutputVertexBuffer(meshIndex).GetResource() },
						D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
						D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				}
			}
#else
			// skinnedMeshなら頂点を書き込み状態に戻す
			if (mesh->IsSkinned()) {

				dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh.get())->GetOutputVertexBuffer(meshIndex).GetResource() },
					D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			}
#endif
		}
	}
}