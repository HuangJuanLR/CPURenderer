#pragma once
#include <glm.hpp>

namespace CPURDR
{
    struct DirectionalLight
    {
        bool enabled = true;
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        float intensity = 1.0f;

        bool castShadows = true;
        float shadowBias = 0.005f;
        float shadowStrength = 1.0f;
        int shadowCascades = 4;
        float shadowDistance = 100.0f;
    };

    struct PointLight
    {
        bool enabled = true;
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        float intensity = 1.0f;

        float range = 10.0f;

        float constantAttenuation = 1.0f;
        float linearAttenuation = 0.09f;
        float quadraticAttenuation = 0.032f;

        bool castShadows = true;
        float shadowBias = 0.005f;
        float shadowStrength = 1.0f;
    };

    struct SpotLight
    {
        bool enabled = true;
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        float intensity = 1.0f;

        float range = 10.0f;

        float innerConeAngle = 25.0f;
        float outerConeAngle = 35.0f;

        float constantAttenuation = 1.0f;
        float linearAttenuation = 0.09f;
        float quadraticAttenuation = 0.032f;

        bool castShadows = true;
        float shadowBias = 0.005f;
        float shadowStrength = 1.0f;
    };

    struct AreaLight
    {
        bool enabled = true;
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        float intensity = 1.0f;

        float width = 1.0f;
        float height = 1.0f;

        bool twoSided = false;
        float range = 20.0f;

        int sampleCount = 16;

        bool castShadows = true;
        float shadowBias = 0.005f;
        float shadowStrength = 1.0f;
    };
}