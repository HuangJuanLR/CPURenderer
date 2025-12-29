#include "Transform.h"

namespace CPURDR
{
	glm::mat4 Transform::GetLocalModelMatrix() const
	{
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
		return translationMatrix * rotationMatrix * scaleMatrix;
	}

	glm::mat4 Transform::GetWorldModelMatrix() const
	{
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), worldPosition);
		glm::mat4 rotationMatrix = glm::mat4_cast(worldRotation);
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), worldScale);
		return translationMatrix * rotationMatrix * scaleMatrix;
	}


	void Transform::SetRotationEuler(float pitch, float yaw, float roll)
	{
		rotation = glm::quat(glm::radians(glm::vec3(pitch, yaw, roll)));
	}

	void Transform::SetEulerAngles(const glm::vec3& euler)
	{
		eulerAngles = euler;
		rotation = glm::quat(glm::radians(euler));
	}

	void Transform::Rotate(float angle, const glm::vec3& axis)
	{
		rotation = glm::rotate(rotation, glm::radians(angle), axis);
		eulerAngles = glm::degrees(glm::eulerAngles(rotation));
	}

	glm::vec3 Transform::GetEulerAngles() const
	{
		return eulerAngles;
	}


}