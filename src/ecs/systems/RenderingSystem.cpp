#include "RenderingSystem.h"

#include "../../Graphics.h"
#include "plog/Log.h"

namespace CPURDR
{
	void RenderingSystem::Render(entt::registry& registry, Context* context, const Camera& camera)
	{
		if (!context)
		{
			PLOG_ERROR << "RenderingSystem::Render() called with null context";
			return;
		}

		auto view = registry.view<Transform, MeshFilter, MeshRenderer>();

		for (auto entity: view)
		{
			const auto& transform = view.get<Transform>(entity);
			const auto& meshFilter = view.get<MeshFilter>(entity);
			const auto& meshRenderer = view.get<MeshRenderer>(entity);

			if (!meshRenderer.enabled)
			{
				continue;
			}

			RenderMeshEntity(transform, meshFilter, meshRenderer, context, camera);
		}
	}

	void RenderingSystem::RenderMeshEntity(const Transform& transform, const MeshFilter& meshFilter, const MeshRenderer& meshRenderer, Context* context, const Camera& camera)
	{
		int width = context->GetFramebufferWidth();
		int height = context->GetFramebufferHeight();
		float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

		Texture2D_RGBA* colorBuffer = context->GetColorBuffer();
		Texture2D_RFloat* depthBuffer = context->GetDepthBuffer();

		if (!colorBuffer || !depthBuffer)
		{
			PLOG_ERROR << "RenderingSystem::RenderMeshEntity(): Context has null framebuffers";
			return;
		}

		glm::mat4 modelMatrix = transform.GetWorldModelMatrix();

		glm::mat4 viewMatrix = camera.GetViewMatrix();
		glm::mat4 projectionMatrix = camera.GetProjectionMatrix(aspectRatio);

		for (const auto& mesh: meshFilter.meshes)
		{
			DrawMesh(mesh, modelMatrix, viewMatrix, projectionMatrix, context, meshRenderer.tint);
		}
	}

	void RenderingSystem::DrawMesh(const Mesh& mesh, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Context* context, uint32_t tint)
	{
		int width = context->GetFramebufferWidth();
		int height = context->GetFramebufferHeight();

		Texture2D_RGBA* colorBuffer = context->GetColorBuffer();
		Texture2D_RFloat* depthBuffer = context->GetDepthBuffer();

		glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

		const auto& vertices = mesh.vertices;
		const auto& indices = mesh.indices;

		for (size_t j = 0, t = 0; j < indices.size(); j+=3, t++)
		{
			glm::vec3 p0 = vertices[indices[j]].position;
			glm::vec3 p1 = vertices[indices[j + 1]].position;
			glm::vec3 p2 = vertices[indices[j + 2]].position;

			glm::vec4 clip0 = mvpMatrix * glm::vec4(p0, 1.0f);
			glm::vec4 clip1 = mvpMatrix * glm::vec4(p1, 1.0f);
			glm::vec4 clip2 = mvpMatrix * glm::vec4(p2, 1.0f);

			if (clip0.w != 0.0f) clip0 /= clip0.w;
			if (clip1.w != 0.0f) clip1 /= clip1.w;
			if (clip2.w != 0.0f) clip2 /= clip2.w;

			if (clip0.x < -1.0f || clip0.x > 1.0f || clip0.y < -1.0f || clip0.y > 1.0f) continue;
			if (clip1.x < -1.0f || clip1.x > 1.0f || clip1.y < -1.0f || clip1.y > 1.0f) continue;
			if (clip2.x < -1.0f || clip2.x > 1.0f || clip2.y < -1.0f || clip2.y > 1.0f) continue;

			if (clip0.z < 0.0f || clip0.z > 1.0f) continue;
			if (clip1.z < 0.0f || clip1.z > 1.0f) continue;
			if (clip2.z < 0.0f || clip2.z > 1.0f) continue;

			glm::vec3 screen0, screen1, screen2;
			screen0.x = (clip0.x + 1.0f) * 0.5f * width;
			screen0.y = (clip0.y + 1.0f) * 0.5f * height;
			screen0.z = clip0.z;
			screen1.x = (clip1.x + 1.0f) * 0.5f * width;
			screen1.y = (clip1.y + 1.0f) * 0.5f * height;
			screen1.z = clip1.z;
			screen2.x = (clip2.x + 1.0f) * 0.5f * width;
			screen2.y = (clip2.y + 1.0f) * 0.5f * height;
			screen2.z = clip2.z;

			const SDL_Color col = (t < mesh.colors.size())? mesh.colors[t] : SDL_Color{255, 255, 255, 255};
			uint32_t color = (col.r << 24) | (col.g << 16) | (col.b << 8) | 255;
			Graphics::Triangle(screen0, screen1, screen2, width, height, *depthBuffer, *colorBuffer, color);
		}
	}


}
