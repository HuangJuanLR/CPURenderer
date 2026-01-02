#include "Gizmos.h"

#include "glm.hpp"
#include "Graphics.h"
#include "ext/matrix_transform.hpp"

namespace CPURDR
{
	void Gizmos::DrawAxis(Context* context, const Camera& camera, float axisLength, float lineThickness)
	{
		if (!context || !context->IsInRenderPass())
		{
			return;
		}

		const glm::vec3 origin(0.0f, 0.0f, 0.f);

		const glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
		const glm::vec3 green = glm::vec3(0.0f, 1.0f, 0.0f);
		const glm::vec3 blue = glm::vec3(0.0f, 0.0f, 1.0f);

		const float arrowHeadSize = axisLength * 0.15f;
		const float arrowHeadRadius = lineThickness * 2.5f;
		const float axisBodyLength = axisLength - arrowHeadSize;

		DrawLine(context, camera, origin, glm::vec3(axisBodyLength, 0.0f, 0.0f), red, lineThickness);
		DrawLine(context, camera, origin, glm::vec3(0.0f, axisBodyLength, 0.0f), green, lineThickness);
		DrawLine(context, camera, origin, glm::vec3(0.0f, 0.0f, axisBodyLength), blue, lineThickness);

		Mesh coneMesh = CreateConeMesh(arrowHeadRadius, arrowHeadSize, 8);

		glm::mat4 xArrowTransform = glm::translate(glm::mat4(1.0f), glm::vec3(axisBodyLength, 0.0f, 0.0f));
		xArrowTransform = glm::rotate(xArrowTransform, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		DrawMesh(context, camera, coneMesh, xArrowTransform, ColorToUint32(red));

		glm::mat4 yArrowTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, axisBodyLength, 0.0f));
		DrawMesh(context, camera, coneMesh, yArrowTransform, ColorToUint32(green));

		glm::mat4 zArrowTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, axisBodyLength));
		zArrowTransform = glm::rotate(zArrowTransform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawMesh(context, camera, coneMesh, zArrowTransform, ColorToUint32(blue));
	}

	Mesh Gizmos::CreateLineMesh(const glm::vec3& start, const glm::vec3& end, float thickness)
	{
		constexpr int segments = 6;
		glm::vec3 direction = end - start;
		float length = glm::length(direction);

		if (length < 0.001f)
		{
			return Mesh({}, {});
		}

		direction = glm::normalize(direction);

		glm::vec3 up = glm::abs(direction.y) < 0.99f? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 right = glm::normalize(glm::cross(up, direction));
		up = glm::normalize(glm::cross(direction, right));

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		for (int i = 0; i <= segments; ++i)
		{
			float angle = (float)i / (float)segments * 2.0f * 3.14159265359f;
			float cosA = std::cos(angle);
			float sinA = std::sin(angle);

			glm::vec3 offset = (right * cosA + up * sinA) * thickness;

			// Start cap vertex
			Vertex v0;
			v0.position = start + offset;
			v0.normal = glm::normalize(offset);
			v0.texcoord = glm::vec2((float)i / segments, 0.0f);
			vertices.push_back(v0);

			// End cap vertex
			Vertex v1;
			v1.position = end + offset;
			v1.normal = glm::normalize(offset);
			v1.texcoord = glm::vec2((float)i / segments, 1.0f);
			vertices.push_back(v1);
		}

		for (int i = 0; i < segments; ++i)
		{
			int curr = i * 2;
			int next = (i + 1) * 2;

			// First triangle
			indices.push_back(curr);
			indices.push_back(next);
			indices.push_back(curr + 1);

			// Second triangle
			indices.push_back(curr + 1);
			indices.push_back(next);
			indices.push_back(next + 1);
		}

		return Mesh(vertices, indices);
	}


	Mesh Gizmos::CreateConeMesh(float radius, float height, int segments)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		Vertex apex;
		apex.position = glm::vec3(0.0f, height, 0.0f);
		apex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
		apex.texcoord = glm::vec2(0.5f, 1.0f);
		vertices.push_back(apex);

		Vertex baseCenter;
		baseCenter.position = glm::vec3(0.0f, 0.0f, 0.0f);
		baseCenter.normal = glm::vec3(0.0f, -1.0f, 0.0f);
		baseCenter.texcoord = glm::vec2(0.5f, 0.0f);
		vertices.push_back(baseCenter);

		for (int i = 0; i <= segments; ++i)
		{
			float angle = (float)i / (float)segments * 2.0f * 3.14159265359f;
			float x = std::cos(angle) * radius;
			float z = std::sin(angle) * radius;

			// Side vertex (for cone surface)
			Vertex v;
			v.position = glm::vec3(x, 0.0f, z);
			v.normal = glm::normalize(glm::vec3(x, radius / height, z)); // Approximate normal
			v.texcoord = glm::vec2((float)i / segments, 0.0f);
			vertices.push_back(v);
		}

		for (int i = 0; i < segments; ++i)
		{
			indices.push_back(0); // Apex
			indices.push_back(2 + i);
			indices.push_back(2 + i + 1);
		}

		for (int i = 0; i < segments; ++i)
		{
			indices.push_back(1); // Base center
			indices.push_back(2 + i + 1);
			indices.push_back(2 + i);
		}

		return Mesh(vertices, indices);
	}


	void Gizmos::DrawMesh(Context* context, const Camera& camera, const Mesh& mesh, const glm::mat4& transform, uint32_t color)
	{
		if (!context || !context->IsInRenderPass())
		{
			return;
		}

		const Viewport& viewport = context->GetViewport();
		int width = viewport.width;
		int height = viewport.height;
		float aspect = (float)viewport.width / (float)viewport.height;

		glm::mat4 viewMatrix = camera.GetViewMatrix();
		glm::mat4 projectionMatrix = camera.GetProjectionMatrix(aspect);
		glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * transform;

		Texture2D_RFloat* depthBuffer = context->GetDepthBuffer();
		Texture2D_RGBA* colorBuffer = context->GetColorBuffer();

		if (!depthBuffer || !colorBuffer)
		{
			return;
		}

		const float EPSILON = 0.001f;

		for (size_t i = 0; i < mesh.indices.size(); i += 3)
		{
			glm::vec3 p0 = mesh.vertices[mesh.indices[i]].position;
			glm::vec3 p1 = mesh.vertices[mesh.indices[i + 1]].position;
			glm::vec3 p2 = mesh.vertices[mesh.indices[i + 2]].position;

			glm::vec4 clip0 = mvpMatrix * glm::vec4(p0, 1.0f);
			glm::vec4 clip1 = mvpMatrix * glm::vec4(p1, 1.0f);
			glm::vec4 clip2 = mvpMatrix * glm::vec4(p2, 1.0f);

			char behindCount = 0;
			if (clip0.w < EPSILON) behindCount++;
			if (clip1.w < EPSILON) behindCount++;
			if (clip2.w < EPSILON) behindCount++;

			if (behindCount == 3) continue;

			if (behindCount > 0)
			{
				ClipAndRenderTriangle(clip0, clip1, clip2, width, height,
					depthBuffer, colorBuffer, color, EPSILON);
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

			Graphics::Triangle(screen0, screen1, screen2,
				width, height, *depthBuffer, *colorBuffer, color);
		}
	}

	void Gizmos::DrawLine(Context* context, const Camera& camera,
		const glm::vec3& start, const glm::vec3& end, const glm::vec3& color, float thickness)
	{
		if (!context || !context->IsInRenderPass())
		{
			return;
		}

		Mesh lineMesh = CreateLineMesh(start, end, thickness);
		DrawMesh(context, camera, lineMesh, glm::mat4(1.0f), ColorToUint32(color));
	}

	// The same as RenderingSystem.cpp
	static glm::vec4 ClipEdge(const glm::vec4& inside, const glm::vec4& outside, float nearPlane)
	{
		float t = (nearPlane - inside.w) / (outside.w - inside.w);
		return inside + t * (outside - inside);
	}

	void Gizmos::ClipAndRenderTriangle(const glm::vec4& v0, const glm::vec4& v1,
		const glm::vec4& v2, int width, int height,
		Texture2D_RFloat* depthBuffer, Texture2D_RGBA* colorBuffer,
		uint32_t color, float nearPlane)
	{
		bool front0 = v0.w >= nearPlane;
		bool front1 = v1.w >= nearPlane;
		bool front2 = v2.w >= nearPlane;

		glm::vec4 clipped[4];
		int clipCount = 0;

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


	uint32_t Gizmos::ColorToUint32(const glm::vec3& color)
	{
		uint8_t r = static_cast<uint8_t>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f);
		uint8_t g = static_cast<uint8_t>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f);
		uint8_t b = static_cast<uint8_t>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f);
		uint8_t a = 255;

		// 0xRRGGBBAA
		return (r << 24) | (g << 16) | (b << 8) | a;
	}


}
