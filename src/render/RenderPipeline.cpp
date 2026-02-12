#include "RenderPipeline.h"
#include <algorithm>

#include "EffectiveMaterial.h"
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

			const Material* baseMaterial = MaterialManager::GetInstance().GetMaterial(meshRenderer.materialId);
			if (baseMaterial == nullptr)
			{
				baseMaterial = MaterialManager::GetInstance().GetMaterial(
					MaterialManager::GetInstance().GetDefaultMaterial());
			}
			if (!baseMaterial) continue;

			// Create effective material with overrides
			Material effectiveMaterial = CreateEffectiveMaterial(*baseMaterial, meshRenderer);

			for (const auto& mesh : meshFilter.meshes)
			{
				DrawMesh(mesh, effectiveMaterial, transform, context);
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
		const Transform& transform, Context* context) const
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
				// Store invW for perspective correction
				float invW0 = 1.0f / v0.positionCS.w;
				float invW1 = 1.0f / v1.positionCS.w;
				float invW2 = 1.0f / v2.positionCS.w;

				v0.positionCS *= invW0;
				v1.positionCS *= invW1;
				v2.positionCS *= invW2;

				v0.positionCS.w = invW0;
				v1.positionCS.w = invW1;
				v2.positionCS.w = invW2;

				bool clipX = (v0.positionCS.x < -1 && v1.positionCS.x < -1 && v2.positionCS.x < -1) ||
							 (v0.positionCS.x > 1 && v1.positionCS.x > 1 && v2.positionCS.x > 1);
				bool clipY = (v0.positionCS.y < -1 && v1.positionCS.y < -1 && v2.positionCS.y < -1) ||
							 (v0.positionCS.y > 1 && v1.positionCS.y > 1 && v2.positionCS.y > 1);
				bool clipZ = (v0.positionCS.z < 0 && v1.positionCS.z < 0 && v2.positionCS.z < 0) ||
							 (v0.positionCS.z > 1 && v1.positionCS.z > 1 && v2.positionCS.z > 1);

				if (clipX || clipY || clipZ) continue;

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

            	// Store invW for perspective correction
            	float invW = 1.0f / v.positionCS.w;
            	v.positionCS *= invW;
            	v.positionCS.w = invW;
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

		// keep a copy for later swap operation
		Varyings pv0 = v0;
		Varyings pv1 = v1;
		Varyings pv2 = v2;

		glm::vec3 s0 = toScreen(pv0.positionCS);
		glm::vec3 s1 = toScreen(pv1.positionCS);
		glm::vec3 s2 = toScreen(pv2.positionCS);

		float area = EdgeFunction(s0, s1, s2);
		if (std::abs(area) < 1e-5f) return;
		//
		// if (area < 0.0f)
		// {
		// 	return;
		// }

		// Bounding box, adjust for pixel center sampling
		int minX = std::max(0, (int)std::ceil(std::min({s0.x, s1.x, s2.x}) - 0.5f));
		int maxX = std::min(width - 1, (int)std::floor(std::max({s0.x, s1.x, s2.x}) - 0.5f));
		int minY = std::max(0, (int)std::ceil(std::min({s0.y, s1.y, s2.y}) - 0.5));
		int maxY = std::min(height - 1, (int)std::floor(std::max({s0.y, s1.y, s2.y}) - 0.5));

		if (minX > maxX || minY > maxY) return;

		float invArea = 1.0f / area;
		const float EPS = 1e-5f;

		// E(x, y) = A * x + B * y + C
		struct EdgeEquation
		{
			float A, B, C;
			bool topLeft;
		};

		auto makeEdgeEquation = [&](const glm::vec3& a, const glm::vec3& b) -> EdgeEquation
		{
			EdgeEquation e;
			e.A = b.y - a.y;
			e.B = a.x - b.x;
			e.C = b.x * a.y - a.x * b.y;

			// Top-Left rule
			// A > 0 means y going down for CCW triangle -> left edge
			// A <= EPS && e.B > 0 -> top edge
			e.topLeft = (e.A > 0 || (std::abs(e.A) <= EPS && e.B > 0));
			return e;
		};

		//  Evaluate edge equation at a given point
		auto eval = [](const EdgeEquation& e, float x, float y) -> float
		{
			return e.A * x + e.B * y + e.C;
		};

		// Test is inside the triangle
		auto isInside = [&](float edgeValue, bool topLeft) -> bool
		{
			// return (edgeValue > EPS) || (std::abs(edgeValue) <= EPS && topLeft);
			return (area > 0)? (edgeValue >= EPS || (std::abs(edgeValue) <= EPS && topLeft)) :
								(edgeValue <= -EPS || (std::abs(edgeValue) <= EPS && topLeft));
		};

		auto packColor = [](const glm::vec4& c) -> uint32_t
		{
			const glm::vec4 clamped = glm::clamp(c, 0.0f, 1.0f);
			uint8_t r = (uint8_t)(clamped.r * 255.0f);
			uint8_t g = (uint8_t)(clamped.g * 255.0f);
			uint8_t b = (uint8_t)(clamped.b * 255.0f);
			uint8_t a = (uint8_t)(clamped.a * 255.0f);
			return r << 24 | g << 16 | b << 8 | a;
		};

		const EdgeEquation e0eq = makeEdgeEquation(s1, s2);
		const EdgeEquation e1eq = makeEdgeEquation(s2, s0);
		const EdgeEquation e2eq = makeEdgeEquation(s0, s1);

		// offset 0.5f to sample pixel center
		const float startX = (float)minX + 0.5f;


		// process in 2x2 blocks
		for (int by = minY; by <= maxY; by+=2)
		{
			const float py = (float)by + 0.5f;

			// Evaluate each edge at the first lane of the row
			float e0Row = eval(e0eq, startX, py);
			float e1Row = eval(e1eq, startX, py);
			float e2Row = eval(e2eq, startX, py);

			for (int bx = minX; bx <= maxX; bx+=2)
			{
				const glm::vec2 p[4] =
				{
					glm::vec2(bx, by),
					glm::vec2(bx + 1, by),
					glm::vec2(bx, by + 1),
					glm::vec2(bx + 1, by + 1)
				};

				const bool valid[4] =
				{
					p[0].x <= maxX && p[0].y <= maxY,
					p[1].x <= maxX && p[1].y <= maxY,
					p[2].x <= maxX && p[2].y <= maxY,
					p[3].x <= maxX && p[3].y <= maxY
				};

				// Increment edge value
				const float e0Lane[4] =
				{
					e0Row,
					e0Row + e0eq.A,
					e0Row + e0eq.B,
					e0Row + e0eq.A + e0eq.B
				};

				const float e1Lane[4] =
				{
					e1Row,
					e1Row + e1eq.A,
					e1Row + e1eq.B,
					e1Row + e1eq.A + e1eq.B
				};

				const float e2Lane[4] =
				{
					e2Row,
					e2Row + e2eq.A,
					e2Row + e2eq.B,
					e2Row + e2eq.A + e2eq.B
				};

				bool anyCovered = false;
				bool covered[4] = {false, false, false, false};

				//
				for (int lane = 0; lane < 4; lane++)
				{
					if (!valid[lane]) continue;
					covered[lane] =
						isInside(e0Lane[lane], e0eq.topLeft) &&
						isInside(e1Lane[lane], e1eq.topLeft) &&
						isInside(e2Lane[lane], e2eq.topLeft);
					anyCovered |= covered[lane];
				}

				// Skip if no cover at all
				if (!anyCovered)
				{
					e0Row += 2.0f * e0eq.A;
					e1Row += 2.0f * e1eq.A;
					e2Row += 2.0f * e2eq.A;
					continue;
				}

				// Shade only covered pixels
				for (int lane = 0; lane < 4; lane++)
				{
					if (!valid[lane] || !covered[lane]) continue;

					float b0 = e0Lane[lane] * invArea;
					float b1 = e1Lane[lane] * invArea;
					float b2 = e2Lane[lane] * invArea;

					float w0 = b0 * pv0.positionCS.w;
					float w1 = b1 * pv1.positionCS.w;
					float w2 = b2 * pv2.positionCS.w;

					float invW = w0 + w1 + w2;
					if (invW <= 0.0f) continue;

					float depth = (w0 * s0.z + w1 * s1.z + w2 * s2.z) * (1.0f / invW);

					if (depth < 0.0f || depth > 1.0f) continue;
					if (depth >= depthBuffer(p[lane].x, p[lane].y)) continue;

					float invInvW = 1.0f / invW;

					Varyings i;
					i.positionWS = (w0 * pv0.positionWS + w1 * pv1.positionWS + w2 * pv2.positionWS) * invInvW;
					i.normalWS = glm::normalize((w0 * pv0.normalWS + w1 * pv1.normalWS + w2 * pv2.normalWS) * invInvW);
					i.uv = (w0 * pv0.uv + w1 * pv1.uv + w2 * pv2.uv) * invInvW;

					glm::vec4 color = shader->Fragment(i, uniforms);

					if (color.a <= 0.0f) continue;

					depthBuffer(p[lane].x, p[lane].y) = depth;
					colorBuffer(p[lane].x, p[lane].y) = packColor(color);
				}

				e0Row += 2.0f * e0eq.A;
				e1Row += 2.0f * e1eq.A;
				e2Row += 2.0f * e2eq.A;
			}
		}
	}

}