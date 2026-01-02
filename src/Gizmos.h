#pragma once
#include "Camera.h"
#include "Model.h"
#include "render/Context.h"

namespace CPURDR
{
	class Gizmos
	{
	public:
		Gizmos() = default;
		~Gizmos() = default;

		static void DrawAxis(Context* context, const Camera& camera, float axisLength = 1.0f, float lineThickness = 0.02f);

		static void DrawLine(Context* context, const Camera& camera,
			const glm::vec3& start, const glm::vec3& end,
			const glm::vec3& color, float thickness = 0.022f);

	private:
		static Mesh CreateLineMesh(const glm::vec3& start, const glm::vec3& end, float thickness = 0.02f);
		static Mesh CreateConeMesh(float radius = 0.05f, float height = 0.15f, int segments = 8);
		static void DrawMesh(Context* context, const Camera& camera, const Mesh& mesh, const glm::mat4& transform, uint32_t color);

		static void ClipAndRenderTriangle(const glm::vec4& v0, const glm::vec4& v1, const glm::vec4& v2,
			int width, int height, Texture2D_RFloat* depthBuffer, Texture2D_RGBA* colorBuffer,
			uint32_t color, float nearPlane);

		static uint32_t ColorToUint32(const glm::vec3& color);

	};
}
