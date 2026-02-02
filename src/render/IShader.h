#pragma once
#include <string>
#include <variant>
#include <vector>

#include "glm.hpp"

#include "ShaderUniforms.h"

namespace CPURDR
{
	struct Material;

	enum class ShaderPropertyType
	{
		Float,
		Int,
		Vec2,
		Vec3,
		Vec4,
		Color,
		Texture,
		Bool
	};

	struct ShaderPropertyDefinition
	{
		std::string name;
		std::string displayName;
		ShaderPropertyType type;

		std::variant<float, int, glm::vec2 , glm::vec3, glm::vec4, uint32_t, bool> defaultValue;

		float minValue = 0.0f;
		float maxValue = 1.0f;
	};

	struct VertexInput
	{
		glm::vec3 positionOS;
		glm::vec3 normalOS;
		glm::vec2 texcoord;
	};

	struct Varyings
	{
		glm::vec4 positionCS;
		glm::vec3 positionWS;
		glm::vec3 normalWS;
		glm::vec2 uv;
	};

	class IShader
	{
	public:
		virtual ~IShader() = default;

		virtual uint32_t GetId() const = 0;
		virtual std::string GetName() const = 0;
		virtual const std::vector<ShaderPropertyDefinition>& GetProperties() const = 0;

		virtual Varyings Vertex(const VertexInput& input, const ShaderUniforms& uniforms) const = 0;
		virtual glm::vec4 Fragment(const Varyings& v, const ShaderUniforms& uniforms) const = 0;
	};
}
