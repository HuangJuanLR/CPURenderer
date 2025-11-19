#include "Transform.h"

namespace CPURDR
{
	glm::mat4 Transform::GetModelMatrix() const
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = model * glm::mat4_cast(rotation);
		model = glm::scale(model, scale);
		return model;
	}

	void Transform::SetRotationEuler(float pitch, float yaw, float roll)
	{
		rotation = glm::quat(glm::radians(glm::vec3(pitch, yaw, roll)));
	}

	void Transform::Rotate(float angle, const glm::vec3& axis)
	{
		rotation = glm::rotate(rotation, glm::radians(angle), axis);
	}

	glm::vec3 Transform::GetEulerAngles() const
	{
		return glm::degrees(glm::eulerAngles(rotation));
	}


}