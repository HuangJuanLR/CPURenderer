#include "RenderPipeline.h"
#include <algorithm>
#include "plog/Log.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "IShader.h"
#include "../Model.h"
#include "../ecs/components/Transform.h"
#include "../ecs/components/MeshFilter.h"
#include "../ecs/components/MeshRenderer.h"
#include "../ecs/components/Light.h"

// ReSharper disable CppCStyleCast

namespace CPURDR
{
	void RenderPipeline::Render(entt::registry& registry,
		Context* context, const Camera& camera)
	{
		if (!context) return;


		float aspectRatio = (float)context->GetFramebufferWidth() / context->GetFramebufferHeight();

		SetupFrameUniforms(registry, camera, aspectRatio);

		RenderOpaqueObject(registry, context);
	}

	void RenderPipeline::SetupFrameUniforms(
		entt::registry& registry,
		const Camera& camera,
		float aspectRatio)
	{
		m_FrameUniforms.viewMatrix = camera.GetViewMatrix();
		m_FrameUniforms.projectionMatrix = camera.GetProjectionMatrix(aspectRatio);
		m_FrameUniforms.viewProjectionMatrix = m_FrameUniforms.projectionMatrix * m_FrameUniforms.viewMatrix;
		m_FrameUniforms.cameraPosition = camera.GetPosition();

		m_FrameUniforms.hasMainLight = false;
		auto dirLightView = registry.view<Transform, DirectionalLight>();
		for (auto entity: dirLightView)
		{
			const auto& transform = dirLightView.get<Transform>(entity);
			const auto& light = dirLightView.get<DirectionalLight>(entity);

			if (!light.enabled) continue;

			m_FrameUniforms.hasMainLight = true;
			m_FrameUniforms.mainLightDirection = transform.worldRotation * glm::vec3(0, 0, -1.0f);
			m_FrameUniforms.mainLightColor = light.color;
			m_FrameUniforms.mainLightIntensity = light.intensity;
			break;
 		}

		m_FrameUniforms.ambientLight = glm::vec3(0.15f);
	}

	void RenderPipeline::RenderOpaqueObject(entt::registry& registry, Context* context)
	{
		auto view = registry.view<Transform, MeshFilter, MeshRenderer>();

		for (auto entity : view)
		{
			const auto& transform = view.get<Transform>(entity);
			const auto& meshFilter = view.get<MeshFilter>(entity);
			const auto& meshRenderer = view.get<MeshRenderer>(entity);

			if (!meshRenderer.enabled) continue;

			const Material* material = MaterialManager::GetInstance().GetMaterial(meshRenderer.materialId);
			if (material == nullptr)
			{
				material = MaterialManager::GetInstance().GetMaterial(
					MaterialManager::GetInstance().GetDefaultMaterial());
			}
			if (!material) continue;

			for (const auto& mesh : meshFilter.meshes)
			{
				DrawMesh(mesh, *material, transform, context);
			}
		}
	}

	Varyings ClipLerpVaryings(const Varyings& inside, const Varyings& outside, float nearPlane)
	{
		float t = (nearPlane - inside.positionCS.w) / (outside.positionCS.w - inside.positionCS.w);

		Varyings result;
		result.positionCS = inside.positionCS + t * (outside.positionCS - inside.positionCS);
		result.positionWS = inside.positionWS + t * (outside.positionWS - inside.positionWS);
		result.normalWS = glm::normalize(inside.normalWS + t * (outside.normalWS - inside.normalWS));
		result.uv = inside.uv + t * (outside.uv - inside.uv);

		return result;
	}

	void RenderPipeline::DrawMesh(const Mesh& mesh, const Material& material,
		const Transform& transform, Context* context)
	{
		ObjectUniforms objectUniforms;
		objectUniforms.objectToWorld = transform.GetWorldModelMatrix();
		objectUniforms.worldToObject = glm::inverse(objectUniforms.objectToWorld);
		objectUniforms.objectToWorldNormal = glm::transpose(glm::mat3(objectUniforms.worldToObject));
		objectUniforms.mvp = m_FrameUniforms.viewProjectionMatrix * objectUniforms.objectToWorld;

		IShader* shader = ShaderManager::GetInstance().GetShader(material.shaderId);
		if (!shader)
		{
			shader = ShaderManager::GetInstance().GetDefaultShader();
		}
		if (!shader) return;

		ShaderUniforms uniforms;
		uniforms.frame = &m_FrameUniforms;
		uniforms.object = &objectUniforms;
		uniforms.material = &material;

		int width = context->GetFramebufferWidth();
		int height = context->GetFramebufferHeight();
		Texture2D_RGBA* colorBuffer = context->GetColorBuffer();
		Texture2D_RFloat* depthBuffer = context->GetDepthBuffer();

		const auto& vertices = mesh.vertices;
		const auto& indices = mesh.indices;
		const float NEAR_PLANE = 0.001;

		for (size_t i = 0; i < indices.size(); i+=3)
		{
			VertexInput v0in =
			{
				vertices[indices[i]].position,
				vertices[indices[i]].normal,
				vertices[indices[i]].texcoord
			};

			VertexInput v1in =
			{
				vertices[indices[i + 1]].position,
				vertices[indices[i + 1]].normal,
				vertices[indices[i + 1]].texcoord
			};

			VertexInput v2in =
			{
				vertices[indices[i + 2]].position,
				vertices[indices[i + 2]].normal,
				vertices[indices[i + 2]].texcoord
			};

			Varyings v0 = shader->Vertex(v0in, uniforms);
			Varyings v1 = shader->Vertex(v1in, uniforms);
			Varyings v2 = shader->Vertex(v2in, uniforms);

			bool front0 = v0.positionCS.w >= NEAR_PLANE;
			bool front1 = v1.positionCS.w >= NEAR_PLANE;
			bool front2 = v2.positionCS.w >= NEAR_PLANE;

			int behindCount = (!front0) + (!front1) + (!front2);

			if (behindCount == 3) continue;

			if (behindCount == 0)
			{
				v0.positionCS /= v0.positionCS.w;
				v1.positionCS /= v1.positionCS.w;
				v2.positionCS /= v2.positionCS.w;

				bool clipX = (v0.positionCS.x < -1 && v1.positionCS.x < -1 && v2.positionCS.x < -1) ||
							 (v0.positionCS.x > 1 && v1.positionCS.x > 1 && v2.positionCS.x > 1);
				bool clipY = (v0.positionCS.y < -1 && v1.positionCS.y < -1 && v2.positionCS.y < -1) ||
							 (v0.positionCS.y > 1 && v1.positionCS.y > 1 && v2.positionCS.y > 1);

				if (clipX || clipY) continue;

				RasterizeTriangle(v0, v1, v2, shader, uniforms, width, height,
					*depthBuffer, *colorBuffer);

				continue;;
			}

			Varyings clipped[4];
            int clipCount = 0;

            // Two vertices clipped (one visible) -> produces 1 triangle
            if (front0 && !front1 && !front2)
            {
                clipped[0] = v0;
                clipped[1] = ClipLerpVaryings(v0, v1, NEAR_PLANE);
                clipped[2] = ClipLerpVaryings(v0, v2, NEAR_PLANE);
                clipCount = 3;
            }
            else if (front1 && !front0 && !front2)
            {
                clipped[0] = v1;
                clipped[1] = ClipLerpVaryings(v1, v0, NEAR_PLANE);
                clipped[2] = ClipLerpVaryings(v1, v2, NEAR_PLANE);
                clipCount = 3;
            }
            else if (front2 && !front0 && !front1)
            {
                clipped[0] = v2;
                clipped[1] = ClipLerpVaryings(v2, v0, NEAR_PLANE);
                clipped[2] = ClipLerpVaryings(v2, v1, NEAR_PLANE);
                clipCount = 3;
            }
            // One vertex clipped (two visible) -> produces 2 triangles (quad)
            else if (front0 && front1 && !front2)
            {
                clipped[0] = v0;
                clipped[1] = v1;
                clipped[2] = ClipLerpVaryings(v1, v2, NEAR_PLANE);
                clipped[3] = ClipLerpVaryings(v0, v2, NEAR_PLANE);
                clipCount = 4;
            }
            else if (front1 && front2 && !front0)
            {
                clipped[0] = v1;
                clipped[1] = v2;
                clipped[2] = ClipLerpVaryings(v2, v0, NEAR_PLANE);
                clipped[3] = ClipLerpVaryings(v1, v0, NEAR_PLANE);
                clipCount = 4;
            }
            else if (front2 && front0 && !front1)
            {
                clipped[0] = v2;
                clipped[1] = v0;
                clipped[2] = ClipLerpVaryings(v0, v1, NEAR_PLANE);
                clipped[3] = ClipLerpVaryings(v2, v1, NEAR_PLANE);
                clipCount = 4;
            }

            // Perspective divide for clipped vertices
            auto perspectiveDivide = [](Varyings& v) {
                v.positionCS /= v.positionCS.w;
            };

            if (clipCount == 3)
            {
                perspectiveDivide(clipped[0]);
                perspectiveDivide(clipped[1]);
                perspectiveDivide(clipped[2]);
                RasterizeTriangle(clipped[0], clipped[1], clipped[2], shader, uniforms,
                                  width, height, *depthBuffer, *colorBuffer);
            }
            else if (clipCount == 4)
            {
                perspectiveDivide(clipped[0]);
                perspectiveDivide(clipped[1]);
                perspectiveDivide(clipped[2]);
                perspectiveDivide(clipped[3]);

                // Render as two triangles
                RasterizeTriangle(clipped[0], clipped[1], clipped[2], shader, uniforms,
                                  width, height, *depthBuffer, *colorBuffer);
                RasterizeTriangle(clipped[0], clipped[2], clipped[3], shader, uniforms,
                                  width, height, *depthBuffer, *colorBuffer);
            }
		}
	}

	inline float EdgeFunction(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& p)
	{
		return (p.x - v0.x) * (v1.y - v0.y) - (p.y - v0.y) * (v1.x - v0.x);
	}

	void RenderPipeline::RasterizeTriangle(
		const Varyings& v0, const Varyings& v1, const Varyings& v2,
		const IShader* shader, const ShaderUniforms& uniforms, int width, int height,
		Texture2D_RFloat& depthBuffer, Texture2D_RGBA& colorBuffer)
	{
		auto toScreen = [&](const glm::vec4& posCS) -> glm::vec3
		{
			return glm::vec3(
				(posCS.x + 1.0f) * 0.5f * width,
				(posCS.y + 1.0f) * 0.5f * height,
				posCS.z
				);
		};

		glm::vec3 s0 = toScreen(v0.positionCS);
		glm::vec3 s1 = toScreen(v1.positionCS);
		glm::vec3 s2 = toScreen(v2.positionCS);

		// Bounding box
		int minX = std::max(0, (int)std::floor(std::min({s0.x, s1.x, s2.x})));
		int maxX = std::min(width - 1, (int)std::ceil(std::max({s0.x, s1.x, s2.x})));
		int minY = std::max(0, (int)std::floor(std::min({s0.y, s1.y, s2.y})));
		int maxY = std::min(height - 1, (int)std::ceil(std::max({s0.y, s1.y, s2.y})));

		float area = EdgeFunction(s0, s1, s2);
		if (std::abs(area) < 0.0001f) return;

		float invArea = 1.0f / area;

		float A01 = s1.y - s0.y;
		float A12 = s2.y - s1.y;
		float A20 = s0.y - s2.y;

		for (int y = minY; y <= maxY; y++)
		{
			glm::vec3 pRow((float)minX, (float)y, 0.0f);
			float e0 = EdgeFunction(s1, s2, pRow);
			float e1 = EdgeFunction(s2, s0, pRow);
			float e2 = EdgeFunction(s0, s1, pRow);

			for (int x = minX; x <= maxX; x++)
			{
				// area > 0 for CCW triangle
				bool inside = (area > 0)? (e0 >= 0 && e1 >= 0 && e2 >= 0)
				: (e0 <= 0 && e1 <= 0 && e2 <= 0);

				if (inside)
				{
					float b0 = e0 * invArea;
					float b1 = e1 * invArea;
					float b2 = e2 * invArea;

					float depth = b0 * s0.z + b1 * s1.z + b2 * s2.z;

					if (depth < depthBuffer(x, y))
					{
						depthBuffer(x, y) = depth;

						Varyings i;
						i.positionWS = b0 * v0.positionWS + b1 * v1.positionWS + b2 * v2.positionWS;
						i.normalWS = glm::normalize(b0 * v0.normalWS + b1 * v1.normalWS + b2 * v2.normalWS);
						i.uv = b0 * v0.uv + b1 * v1.uv + b2 * v2.uv;

						glm::vec4 color = shader->Fragment(i, uniforms);

						uint8_t r = (uint8_t)(color.r * 255.0f);
						uint8_t g = (uint8_t)(color.g * 255.0f);
						uint8_t b = (uint8_t)(color.b * 255.0f);
						uint8_t a = (uint8_t)(color.a * 255.0f);

						colorBuffer(x, y) = (r << 24) | (g << 16) | (b << 8) | a;
					}
				}

				e0 += A12;
				e1 += A20;
				e2 += A01;
			}
		}
	}

}