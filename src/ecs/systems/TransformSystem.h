#pragma once
#include "entt.hpp"
#include "glm.hpp"

#include "../components/Transform.h"

namespace CPURDR
{
	class TransformSystem
	{
	public:
		TransformSystem() = default;
		~TransformSystem() = default;

		void Update(entt::registry& registry);
	private:
		// Update transforms recursively
		void UpdateTransformHierarchy(
			entt::registry& registry,
			entt::entity entity,
			const glm::mat4& parentWorldMatrix = glm::mat4(1.0f)
		);

		void UpdateWorldTransform(
			Transform& transform,
			const glm::mat4& parentWorldMatrix
		);
	};
}
