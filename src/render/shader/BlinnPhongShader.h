#pragma once
#include <algorithm>
#include "../IShader.h"
#include "../Material.h"

namespace CPURDR
{
	class BlinnPhongShader: public IShader
	{
	public:
		static constexpr uint32_t Id = 1;

		uint32_t GetId() const override { return Id; };
		std::string GetName() const override {return "Blinn-Phong";}

		const std::vector<ShaderPropertyDefinition>& GetProperties() const override
		{
			static std::vector<ShaderPropertyDefinition> props = {
				{"_BaseColor", "Base Color", ShaderPropertyType::Color, glm::vec3(0.8f), 0.0f, 1.0f},
				{"_SpecularColor", "Specular", ShaderPropertyType::Color, glm::vec3(1.0f), 0.0f, 1.0f},
				{"_Shininess", "Shininess", ShaderPropertyType::Float, 32.0f, 1.0f, 256.0f},
			};
			return props;
		}

		Varyings Vertex(const VertexInput& input, const ShaderUniforms& uniforms) const override
		{
			Varyings o;
			glm::vec4 positionWS = uniforms.object->objectToWorld * glm::vec4(input.positionOS, 1.0f);
			o.positionWS = glm::vec3(positionWS);
			o.normalWS = glm::normalize(uniforms.object->objectToWorldNormal * input.normalOS);
			o.positionCS = uniforms.object->mvp * glm::vec4(input.positionOS, 1.0f);
			o.uv = input.texcoord;

			return o;
		}

		glm::vec4 Fragment(const Varyings& v, const ShaderUniforms& uniforms) const override
		{
			glm::vec3 baseColor = uniforms.material->GetVec3("_BaseColor", glm::vec3(0.8f));
			glm::vec3 specColor = uniforms.material->GetVec3("_SpecularColor", glm::vec3(1.0f));
			float shininess = uniforms.material->GetFloat("_Shininess", 32.0f);

			glm::vec3 N = normalize(v.normalWS);
			glm::vec3 V = normalize(uniforms.frame->cameraPosition - v.positionWS);

			glm::vec3 color = uniforms.frame->ambientLight * baseColor;

			if (uniforms.frame->hasMainLight)
			{
				glm::vec3 L = glm::normalize(-uniforms.frame->mainLightDirection);
				float NoL = std::max(glm::dot(N, L), 0.0f);

				color += uniforms.frame->mainLightColor * uniforms.frame->mainLightIntensity * baseColor * NoL;

				if (NoL > 0.0f)
				{
					glm::vec3 H = normalize(L + V);
					float NoH = std::max(glm::dot(N, H), 0.0f);
					float spec = std::pow(NoH, shininess);
					color += uniforms.frame->mainLightColor * uniforms.frame->mainLightIntensity * specColor * spec;
				}
			}

			color = glm::clamp(color, glm::vec3(0.0f), glm::vec3(1.0f));
			return glm::vec4(color, 1.0f);
		}
	};
}