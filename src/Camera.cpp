#include "Camera.h"

#include <algorithm>

#include "glm.hpp"
#include "ext/matrix_transform.hpp"
#include "ext/matrix_clip_space.hpp"

namespace CPURDR
{
	Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch):
		m_Position(position),
		m_WorldUp(up),
		m_Yaw(yaw),
		m_Pitch(pitch),
		m_Roll(0.0f),
		m_MovementSpeed(DEFAULT_SPEED),
		m_MouseSensitivity(DEFAULT_SENSITIVITY),
		m_FOV(DEFAULT_FOV),
		m_Near(DEFAULT_NEAR),
		m_Far(DEFAULT_FAR)
	{
		UpdateCameraVectors();
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		// R-Hand match OpenGL
		// return glm::lookAt(m_Position, m_Position + m_Front, m_Up);

		// ===================================
		// construct manually
		// ===================================
		glm::vec3 f = glm::normalize(m_Front);
		glm::vec3 r = glm::normalize(m_Right);
		glm::vec3 u = glm::normalize(m_Up);

		// Camera looks down the -Z-axis(OpenGL convention)
		glm::mat4 view = glm::mat4(
			r.x, u.x, -f.x, 0.0f,
			r.y, u.y, -f.y, 0.0f,
			r.z, u.z, -f.z, 0.0f,
			-glm::dot(r, m_Position), -glm::dot(u, m_Position), glm::dot(f, m_Position), 1.0f
			);

		return view;
	}

	glm::mat4 Camera::GetProjectionMatrix(float aspect) const
	{
		// glm::mat4 projection = glm::perspective(glm::radians(m_FOV), aspect, m_Near, m_Far);
		// projection[1][1] *= 1.0f;
		// projection[2][3] *= -1.0f;
		// return projection;

		// aspect = width / height
		float f = 1.0f / tan(glm::radians(m_FOV) * 0.5f);

		// ================================
		// OpenGL Convention
		// ================================
		// glm::mat4 projection = glm::mat4(
		// 	f / aspect, 0.0f, 0.0f, 0.0f,
		// 	0.0f, f, 0.0f, 0.0f,
		// 	0.0f, 0.0f, -(m_Far + m_Near) / (m_Far - m_Near), 1.0f,
		// 	0.0f, 0.0f, -2.0 * (m_Far * m_Near) / (m_Far - m_Near), 0.0f
		// 	);

		// row-major format so I can viz if better
		// f / aspect, 0.0f, 0.0f,                                 0.0f,
		// 0.0f,       f,    0.0f,                                 0.0f,
		// 0.0f,       0.0f, -(m_Far + m_Near) / (m_Far - m_Near), -2.0 * (m_Far * m_Near) / (m_Far - m_Near),
		// 0.0f,       0.0f, 1.0,                                  0.0f

		// ================================
		// Vulkan Convention
		// ================================
		glm::mat4 projection = glm::mat4(
			f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, -f, 0.0f, 0.0f,
			0.0f, 0.0f, m_Far / (m_Near - m_Far), -1.0f,
			0.0f, 0.0f, (m_Far * m_Near) / (m_Near - m_Far), 0.0f
			);

		// row-major format so I can viz if better
		// f / aspect, 0.0f, 0.0f,                      0.0f,
		// 0.0f,       -f,    0.0f,                      0.0f,
		// 0.0f,       0.0f, m_Far / (m_Near - m_Far),  (m_Far * m_Near) / (m_Near - m_Far),
		// 0.0f,       0.0f, -1.0,                      0.0f

		return projection;
	}

	glm::mat4 Camera::GetViewProjectionMatrix(float aspect) const
	{
		return GetProjectionMatrix(aspect) * GetViewMatrix();
	}

	void Camera::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
	}

	void Camera::SetRotation(float yaw, float pitch, float roll)
	{
		m_Yaw = yaw;
		m_Pitch = pitch;
		m_Roll = roll;

		m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);
		UpdateCameraVectors();
	}

	void Camera::SetFOV(float fov)
	{
		m_FOV = std::clamp(fov, 1.0f, 179.0f);
	}

	void Camera::SetClipPlanes(float nearPlane, float farPlane)
	{
		m_Near = nearPlane;
		m_Far = farPlane;
	}

	void Camera::LookAt(const glm::vec3& target)
	{
		glm::vec3 direction = glm::normalize(target - m_Position);

		m_Pitch = glm::degrees(asin(direction.y));
		m_Yaw = glm::degrees(atan2(direction.z, direction.x));

		UpdateCameraVectors();
	}

	void Camera::Reset()
	{
		m_Position = glm::vec3(0.0f, 0.0f, 3.0f);
		m_Yaw = DEFAULT_YAW;
		m_Pitch = DEFAULT_PITCH;
		m_Roll = 0.0f;
		m_FOV = DEFAULT_FOV;
		m_Near = DEFAULT_NEAR;
		m_Far = DEFAULT_FAR;
		UpdateCameraVectors();
	}

	glm::vec3 Camera::WorldToScreen(const glm::vec3& worldPos, int screenWidth, int screenHeight, float aspectRatio) const
	{
		// World to Clip
		glm::mat4 viewProj = GetViewProjectionMatrix(aspectRatio);
		glm::vec4 clipPos = viewProj * glm::vec4(worldPos, 1.0f);

		// Perspective division
		if (clipPos.w != 0.0f)
		{
			clipPos /= clipPos.w;
		}

		glm::vec3 screenPos;
		screenPos.x = (clipPos.x + 1.0f) * 0.5f * screenWidth;
		screenPos.y = (1.0f - clipPos.y) * 0.5f * screenHeight;
		screenPos.z = clipPos.z;

		return screenPos;
	}

	glm::vec3 Camera::ScreenToWorld(const glm::vec3& screenPos, int screenWidth, int screenHeight, float aspectRatio) const
	{
		glm::vec4 clipPos;
		clipPos.x = (screenPos.x / screenWidth) * 2.0f - 1.0f;
		clipPos.y = 1.0 - (screenPos.y / screenHeight) * 2.0f;
		clipPos.z = screenPos.z;

		glm::mat4 invViewProj = glm::inverse(GetViewProjectionMatrix(aspectRatio));
		glm::vec4 worldPos = invViewProj * clipPos;

		if (worldPos.w != 0.0f)
		{
			worldPos /= worldPos.w;
		}

		return glm::vec3(worldPos);
	}

	bool Camera::IsPointInFrustum(const glm::vec3& point, const float aspectRatio) const
	{
		glm::vec3 toPoint = point - m_Position;

		// Check if the point is within the near and far planes
		float distance = glm::dot(toPoint, m_Front);
		if (distance < m_Near || distance > m_Far)
		{
			return false;
		}

		float halfFOV = glm::radians(m_FOV * 0.5f);

		// Vertical FOV
		float upDist = glm::dot(toPoint, m_Up);
		float maxUp = distance * tan(halfFOV);
		if (abs(upDist) > maxUp)
		{
			return false;
		}

		// Horizontal FOV
		float rightDist = glm::dot(toPoint, m_Right);
		float maxRight = maxUp * aspectRatio;
		if (abs(rightDist) > maxRight)
		{
			return false;
		}

		return true;
	}

	void Camera::UpdateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		front.y = sin(glm::radians(m_Pitch));
		front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_Front = glm::normalize(front);

		m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
		m_Up = glm::normalize(glm::cross(m_Right, m_Front));

		if (m_Roll != 0.0f)
		{
			glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_Roll), m_Front);
			m_Up = glm::vec3(rollMatrix * glm::vec4(m_Up, 0.0f));
			m_Right = glm::normalize(glm::cross(m_Front, m_Up));
		}
	}

}