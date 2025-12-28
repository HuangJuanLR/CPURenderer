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

			const float EPSILON = 0.001f;

			char behindCount = 0;
			if (clip0.w < EPSILON) behindCount++;
			if (clip1.w < EPSILON) behindCount++;
			if (clip2.w < EPSILON) behindCount++;

			if (behindCount == 3) continue;

			if (behindCount > 0)
			{
				ClipAndRenderTriangle(clip0, clip1, clip2, width, height, depthBuffer, colorBuffer,
					(t < mesh.colors.size())? mesh.colors[t] : SDL_Color{255, 255, 255, 255}, EPSILON);

				continue;
			}

			clip0 /= clip0.w;
			clip1 /= clip1.w;
			clip2 /= clip2.w;

			bool shouldClipX = (clip0.x < -1.0f && clip1.x < -1.0f && clip2.x < -1.0f) ||
				(clip0.x > 1.0f && clip1.x > 1.0f && clip2.x > 1.0f);
			bool shouldClipY = (clip0.y < -1.0f && clip1.y < -1.0f && clip2.y < -1.0f) ||
				(clip0.y > 1.0f && clip1.y > 1.0f && clip2.y > 1.0f);

			if (shouldClipX || shouldClipY) continue;

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

	glm::vec4 ClipEdge(const glm::vec4& inside, const glm::vec4& outside, float nearPlane)
	{
		float t = (nearPlane - inside.w) / (outside.w - inside.w);
		return inside + t * (outside - inside);
	}

	void RenderingSystem::ClipAndRenderTriangle(const glm::vec4& v0, const glm::vec4& v1, const glm::vec4& v2,
		int width, int height, Texture2D_RFloat* depthBuffer, Texture2D_RGBA* colorBuffer, const SDL_Color& col,
		float nearPlane)
	{
		bool front0 = v0.w >= nearPlane;
		bool front1 = v1.w >= nearPlane;
		bool front2 = v2.w >= nearPlane;

		glm::vec4 clipped[4];
		int clipCount = 0;

		// Two vertices clipped
		if (front0 && !front1 && !front2)
		{
			clipped[0] = v0;
			clipped[1] = ClipEdge(v0, v1, nearPlane);
			clipped[2] = ClipEdge(v0, v2, nearPlane);
			clipCount = 3;
		}
		else if (front1 && !front0 && !front2)
		{
			clipped[0] = v1;
			clipped[1] = ClipEdge(v1, v0, nearPlane);
			clipped[2] = ClipEdge(v1, v2, nearPlane);
			clipCount = 3;
		}
		else if (front2 && !front0 && !front1)
		{
			clipped[0] = v2;
			clipped[1] = ClipEdge(v2, v0, nearPlane);
			clipped[2] = ClipEdge(v2, v1, nearPlane);
			clipCount = 3;
		}
		// One vertex clipped
		else if (front0 && front1 && !front2)
		{
			clipped[0] = v0;
			clipped[1] = v1;
			clipped[2] = ClipEdge(v1, v2, nearPlane);
			clipped[3] = ClipEdge(v0, v2, nearPlane);
			clipCount = 4;
		}
		else if (front1 && front2 && !front0)
		{
			clipped[0] = v1;
			clipped[1] = v2;
			clipped[2] = ClipEdge(v2, v0, nearPlane);
			clipped[3] = ClipEdge(v1, v0, nearPlane);
			clipCount = 4;
		}
		else if (front2 && front0 && !front1)
		{
			clipped[0] = v2;
			clipped[1] = v0;
			clipped[2] = ClipEdge(v0, v1, nearPlane);
			clipped[3] = ClipEdge(v2, v1, nearPlane);
			clipCount = 4;
		}

		uint32_t color = (col.r << 24) | (col.g << 16) | (col.b << 8) | 255;

		auto ClipToScreen = [&](const glm::vec4& v) -> glm::vec3
		{
			glm::vec3 ndc = v / v.w;
			return glm::vec3(
				(ndc.x + 1.0f) * 0.5f * width,
				(ndc.y + 1.0f) * 0.5f * height,
				ndc.z
				);
		};

		if (clipCount == 3)
		{
			glm::vec3 s0 = ClipToScreen(clipped[0]);
			glm::vec3 s1 = ClipToScreen(clipped[1]);
			glm::vec3 s2 = ClipToScreen(clipped[2]);
			Graphics::Triangle(s0, s1, s2, width, height, *depthBuffer, *colorBuffer, color);
		}
		else if (clipCount == 4)
		{
			glm::vec3 s0 = ClipToScreen(clipped[0]);
			glm::vec3 s1 = ClipToScreen(clipped[1]);
			glm::vec3 s2 = ClipToScreen(clipped[2]);
			glm::vec3 s3 = ClipToScreen(clipped[3]);

			Graphics::Triangle(s0, s1, s2, width, height, *depthBuffer, *colorBuffer, color);
			Graphics::Triangle(s0, s2, s3, width, height, *depthBuffer, *colorBuffer, color);
		}
	}
}
