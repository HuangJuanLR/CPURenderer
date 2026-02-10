#pragma once
#include "../IShader.h"
#include "../Material.h"

namespace CPURDR
{
	class PBRShader: public IShader
	{
	public:
		static constexpr uint32_t Id = 3;
		static constexpr float PI = 3.14159265359f;

		uint32_t GetId() const override {return Id;}
		std::string GetName() const override { return "PBR"; }

		const std::vector<ShaderPropertyDefinition>& GetProperties() const override
		{
			static std::vector<ShaderPropertyDefinition> props = {
				{"_Albedo", "Albedo", ShaderPropertyType::Color, glm::vec3(0.8f), 0.0f, 1.0f},
				{"_Metallic", "Metallic", ShaderPropertyType::Float, 0.0f, 0.0f, 1.0f},
				{"_Roughness", "Roughness", ShaderPropertyType::Float, 0.5f, 0.01f, 1.0f},
				{"_AO", "Ambient Occlusion", ShaderPropertyType::Float, 1.0f, 0.0f, 1.0f},
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
			glm::vec3 albedo = uniforms.material->GetVec3("_Albedo", glm::vec3(0.8f));
			float metallic = uniforms.material->GetFloat("_Metallic", 0.0f);
			float roughness = uniforms.material->GetFloat("_Roughness", 0.5f);
			float ao = uniforms.material->GetFloat("_AO", 1.0f);

			roughness = std::max(roughness, 0.01f);

			glm::vec3 N = glm::normalize(v.normalWS);
			glm::vec3 V = glm::normalize(uniforms.frame->cameraPosition - v.positionWS);

			float NoV = std::max(glm::dot(N, V), 0.0f);

			glm::vec3 F0 = glm::vec3(0.04f);
			F0 = glm::mix(F0, albedo, metallic);

			glm::vec3 Lo = glm::vec3(0.0f);

			if (uniforms.frame->hasMainLight)
			{
				glm::vec3 L = glm::normalize(uniforms.frame->mainLightDirection);
				glm::vec3 H = glm::normalize(L + V);

				float NoL = std::max(glm::dot(N, L), 0.0f);
				float NoH = std::max(glm::dot(N, H), 0.0f);
				float HoV = std::max(glm::dot(H, V), 0.0f);

				glm::vec3 radiance = uniforms.frame->mainLightColor * uniforms.frame->mainLightIntensity;

				float D = DistributionGGX(NoH, roughness);
				float G = GeometrySmith(NoV, NoL, roughness);
				glm::vec3 F = FresnelSchlick(HoV, F0);

				glm::vec3 nom = D * G * F;
				float denom = 4.0f * NoL * NoV + 0.0001f;
				glm::vec3 specular = nom / denom;

				// portion of light gets reflected
				glm::vec3 kS = F;
				// portion of light gets refracted
				glm::vec3 kD = (glm::vec3(1.0f) - kS) * (1.0f - metallic);

				// Lambert diffuse
				glm::vec3 diffuse = kD * albedo / PI;

				Lo += (diffuse + specular) * radiance * NoL;
			}

			glm::vec3 ambient = uniforms.frame->ambientLight * albedo * ao;

			glm::vec3 color = Lo + ambient;

			color = color / (color + glm::vec3(1.0f));
			// gamma correction
			color = glm::pow(color, glm::vec3(1.0f / 2.2f));
			return glm::vec4(glm::clamp(color, 0.0f, 1.0f), 1.0f);
		}

	private:
		//                    α²
		// D(h) = ────────────────────────────
		//        π × ((n·h)² × (α² - 1) + 1)²
		float DistributionGGX(float NoH, float roughness) const
		{
			float a = roughness * roughness;
			float a2 = a * a;
			float NoH2 = NoH * NoH;
			float nom = a2;
			float denom = (NoH2 * (a2 - 1.0f) + 1.0f);
			denom = PI * denom * denom;
			return nom / denom;
		}

		//                    n·v
		// G₁(n,v,k) = ─────────────────
		//              (n·v)(1-k) + k
		//
		// k = (roughness + 1)² / 8  (for direct lighting)
		//
		// G(n,v,k) * G(n,l,k)
		float GeometrySchlickGGX(float NoX, float k) const
		{
			float nom = NoX;
			float denom = NoX * (1.0f - k) + k;
			return nom / denom;
		}

		float GeometrySmith(float NoV, float NoL, float roughness) const
		{
			float r = roughness + 1.0f;
			float k = r * r / 8.0f;

			float ggx1 = GeometrySchlickGGX(NoV, k);
			float ggx2 = GeometrySchlickGGX(NoL, k);

			return ggx1 * ggx2;
		}

		//
		// F(h,v,F₀) = F₀ + (1 - F₀) × (1 - h·v)⁵
		//
		glm::vec3 FresnelSchlick(float HoV, const glm::vec3& F0) const
		{
			float factor = std::pow(1.0f - HoV, 5.0f);
			return F0 + (glm::vec3(1.0f) - F0) * factor;
		}
	};
}