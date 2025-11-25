#pragma once
#include "../../render/Context.h"
#include "entt.hpp"
#include "../../Camera.h"
#include "../components/MeshFilter.h"
#include "../components/MeshRenderer.h"
#include "../components/Transform.h"

namespace CPURDR
{
	class RenderingSystem
	{
	public:
		RenderingSystem() = default;
		~RenderingSystem() = default;

		void Render(entt::registry& registry, Context* context, const Camera& camera);
	private:
		void RenderMeshEntity(
			const Transform& transform,
			const MeshFilter& meshFilter,
			const MeshRenderer& meshRenderer,
			Context* context,
			const Camera& camera
		);

		void DrawMesh(
			const Mesh& mesh,
			const glm::mat4& modelMatrix,
			const glm::mat4& viewMatrix,
			const glm::mat4& projectionMatrix,
			Context* context,
			uint32_t tint
		);
	};
}
