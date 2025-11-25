#pragma once
#include "glm.hpp"
#include "gtc/quaternion.hpp"

namespace CPURDR
{
	struct Transform
	{
		// Local transform(relative to parent)
		glm::vec3 position = glm::vec3(0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // identity
		glm::vec3 scale = glm::vec3(1.0f);

		// World transform
		glm::vec3 worldPosition = glm::vec3(0.0f);
		glm::quat worldRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 worldScale = glm::vec3(1.0f);

		bool isDirty = true;

		Transform() = default;
		Transform(
			const glm::vec3& position,
			const glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			const glm::vec3& scale = glm::vec3(1.0f)
			): position(position), rotation(rotation), scale(scale),
			worldPosition(position), worldRotation(rotation), worldScale(scale){}

		glm::mat4 GetLocalModelMatrix() const;

		glm::mat4 GetWorldModelMatrix() const;

		void SetRotationEuler(float pitch, float yaw, float roll);
		void Rotate(float angle, const glm::vec3& axis);
		glm::vec3 GetEulerAngles() const;

		void MarkDirty() {isDirty = true;}
	};
}
