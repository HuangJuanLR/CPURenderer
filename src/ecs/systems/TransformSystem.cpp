#include "TransformSystem.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/matrix_decompose.hpp"

#include "../components/Hierarchy.h"

namespace CPURDR
{
	void TransformSystem::Update(entt::registry& registry)
	{
		auto transformView = registry.view<Transform>();

		for (auto entity: transformView)
		{
			auto* hierarchy = registry.try_get<Hierarchy>(entity);

			if (!hierarchy || !hierarchy->HasParent())
			{
				UpdateTransformHierarchy(registry, entity, glm::mat4(1.0f));
			}
		}
	}

	void TransformSystem::UpdateTransformHierarchy(entt::registry& registry, entt::entity entity, const glm::mat4& parentWorldMatrix)
	{
		if (!registry.valid(entity)) return;

		auto* transform = registry.try_get<Transform>(entity);
		if (!transform) return;

		UpdateWorldTransform(*transform, parentWorldMatrix);

		glm::mat4 currentWorldMatrix = transform->GetWorldModelMatrix();

		auto* hierarchy = registry.try_get<Hierarchy>(entity);
		if (hierarchy && hierarchy->HasChildren())
		{
			for (entt::entity child: hierarchy->children)
			{
				UpdateTransformHierarchy(registry, child, currentWorldMatrix);
			}
		}
	}

	void TransformSystem::UpdateWorldTransform(Transform& transform, const glm::mat4& parentWorldMatrix)
	{
		if (!transform.isDirty) return;

		glm::mat4 localMatrix = transform.GetLocalModelMatrix();
		glm::mat4 worldMatrix = parentWorldMatrix * localMatrix;

		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(
			worldMatrix,
			transform.worldScale,
			transform.worldRotation,
			transform.worldPosition,
			skew,
			perspective
		);

		transform.isDirty = false;
	}


}
