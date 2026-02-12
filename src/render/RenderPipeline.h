#pragma once
#include "entt.hpp"

#include "Context.h"
#include "IShader.h"
#include "../Camera.h"

namespace CPURDR
{
	struct Mesh;
	struct Material;
	struct Transform;
	class IShader;

	class RenderPipeline
	{
	public:
		RenderPipeline() = default;
		~RenderPipeline() = default;

		void Render(entt::registry& registry, Context* context, const Camera& camera);

	private:
		void SetupFrameUniforms(entt::registry& registry, const Camera& camera, float aspectRatio);
		void RenderOpaqueObject(entt::registry& registry, Context* context);

		void DrawMesh(const Mesh& mesh, const Material& material, const Transform& transform, Context* context) const;
		static void RasterizeTriangle(
			const Varyings& v0, const Varyings& v1, const Varyings& v2,
			const IShader* shader, const ShaderUniforms& uniforms,
			int width, int height,
			Texture2D_RFloat& depthBuffer, Texture2D_RGBA& colorBuffer
			);

		FrameUniforms m_FrameUniforms;
	};
}