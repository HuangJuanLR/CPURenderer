#pragma once
#include <glm.hpp>
#include <array>

namespace CPURDR
{
	static constexpr int MAX_ADDITIONAL_LIGHTS = 8;

	struct LightData
	{
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 color = glm::vec3(1.0f);
		float intensity = 1.0f;
		float range = 10.0f;
		int type = 0;
	};

	struct FrameUniforms
	{
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);
		glm::vec3 cameraPosition = glm::vec3(0.0f);

		bool hasMainLight = false;
		glm::vec3 mainLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 mainLightColor = glm::vec3(1.0f);
		float mainLightIntensity = 1.0f;

		int additionalLightsCount = 0;
		std::array<LightData, MAX_ADDITIONAL_LIGHTS> additionalLights;

		glm::vec3 ambientLight = glm::vec3(0.1f);
		float time = 0.0f;
	};

	struct ObjectUniforms
	{
		glm::mat4 objectToWorld = glm::mat4(1.0f);
		glm::mat4 worldToObject = glm::mat4(1.0f);
		glm::mat3 objectToWorldNormal = glm::mat3(1.0f);
		glm::mat4 mvp = glm::mat4(1.0f);
	};

	struct Material;

	struct ShaderUniforms
	{
		const FrameUniforms* frame = nullptr;
		const ObjectUniforms* object = nullptr;
		const Material* material = nullptr;
	};
}