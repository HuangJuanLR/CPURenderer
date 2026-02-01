#pragma once
#include <glm.hpp>
#include <algorithm>
#include "../IShader.h"
#include "../Material.h"

namespace CPURDR
{
	class UnlitShader: public IShader
	{
	public:
		static constexpr uint32_t Id = 2;

		uint32_t GetId() const override {return Id;}
		std::string GetName() const override {return "Unlit";}

		const std::vector<ShaderPropertyDefinition>& GetProperties() const override
		{
			static std::vector<ShaderPropertyDefinition> props = {
				{"_Color", "Color", ShaderPropertyType::Color, glm::vec3(1.0f), 0.0f, 1.0f},
			};
			return props;
		}

		Varyings Vertex(const VertexInput& input, const ShaderUniforms& uniforms) const override
		{
			Varyings o;
			o.positionWS = glm::vec3(uniforms.object->objectToWorld * glm::vec4(input.positionOS, 1.0f));
			o.normalWS = input.normalOS; // normal is useless here
			o.positionCS = uniforms.object->mvp * glm::vec4(input.positionOS, 1.0f);
			o.uv = input.texcoord;
			return o;
		}

		glm::vec4 Fragment(const Varyings& v, const ShaderUniforms& uniforms) const override
		{
			glm::vec3 color = uniforms.material->GetVec3("_Color", glm::vec3(1.0f));
			return glm::vec4(color, 1.0f);
		}

	};
}
