#include "AnimationSystem.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/ECS/Core/EntityManager.h>
#include <Engine/Core/ECS/Components/AnimationComponent.h>

//============================================================================
//	AnimationSystem classMethods
//============================================================================

Archetype AnimationSystem::Signature() const {

	Archetype arch{};
	arch.set(EntityManager::GetTypeID<AnimationComponent>());
	return arch;
}

void AnimationSystem::Update(EntityManager& entityManager) {

	for (uint32_t entity : entityManager.View(Signature())) {

		auto* animation = entityManager.GetComponent<AnimationComponent>(entity);
		auto* transform = entityManager.GetComponent<Transform3DComponent>(entity);
		animation->Update(transform->matrix.world);
	}
}