#pragma once
#include "glm.hpp"
#include "gtc/quaternion.hpp"

namespace CPURDR
{
	struct Transform
	{
		glm::vec3 position = glm::vec3(0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // identity
		glm::vec3 scale = glm::vec3(1.0f);

		Transform() = default;
		Transform(
			const glm::vec3& position,
			const glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			const glm::vec3& scale = glm::vec3(1.0f)
			): position(position), rotation(rotation), scale(scale){}

		glm::mat4 GetModelMatrix() const;
		void SetRotationEuler(float pitch, float yaw, float roll);
		void Rotate(float angle, const glm::vec3& axis);
		glm::vec3 GetEulerAngles() const;
	};
}
