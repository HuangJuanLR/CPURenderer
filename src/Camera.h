#pragma once
#include "fwd.hpp"
#include "vec3.hpp"

namespace CPURDR
{
	// Vulkan specifications
	// ===============
	// NDC
	// ===============
	// X: -1.0 ~ 1.0(left to right)
	// Y: -1.0 ~ 1.0(top to bottom)
	// Z: 0.0 ~ 1.0(near to far)

	// ===============
	// Framebuffer Coord
	// ===============
	// Origin at top-left, Y-Down

	// ===============
	// Front-face Winding
	// ===============
	// Default is counter-clockwise, can be configured via VKPipelineRasterizationStateCreateInfo
	class Camera
	{
	public:
		Camera(glm::vec3 position = glm::vec3{0.0f, 0.0f, 3.0f},
			glm::vec3 up = glm::vec3{0.0f, 1.0f, 0.0f},
			float yaw = -90.0f,
			float pitch = 0.0f
			);
		~Camera() = default;

		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjectionMatrix(float aspect) const;
		glm::mat4 GetViewProjectionMatrix(float aspect) const;

		glm::vec3 GetPosition() const {return m_Position;}
		glm::vec3 GetFront() const { return m_Front; }
		glm::vec3 GetUp() const { return m_Up; }
		glm::vec3 GetRight() const { return m_Right; }
		glm::vec3 GetWorldUp() const { return m_WorldUp; }

		float GetYaw() const { return m_Yaw; }
		float GetPitch() const { return m_Pitch; }
		float GetRoll() const { return m_Roll; }
		float GetFOV() const { return m_FOV; }
		float GetNear() const { return m_Near; }
		float GetFar() const { return m_Far; }

		void SetPosition(const glm::vec3& position);
		void SetRotation(float yaw, float pitch, float roll = 0.0f);
		void SetFOV(float fov);
		void SetClipPlanes(float nearPlane, float farPlane);

		void LookAt(const glm::vec3& target);
		void Reset();

		glm::vec3 WorldToScreen(const glm::vec3& worldPos, int screenWidth, int screenHeight, float aspectRatio) const;
		glm::vec3 ScreenToWorld(const glm::vec3& screenPos, int screenWidth, int screenHeight, float aspectRatio) const;
		bool IsPointInFrustum(const glm::vec3& point, const float aspectRatio) const;

		void ProcessKeyboard(float deltaTime, bool forward, bool backward, bool left, bool right, bool sprint = false);
		void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
		void SetMovementSpeed(float speed){m_MovementSpeed = speed;}
		void SetMouseSensitivity(float sensitivity){m_MouseSensitivity = sensitivity;}
		float GetMovementSpeed() const {return m_MovementSpeed;}
		float GetMouseSensitivity() const {return m_MouseSensitivity;}

	private:
		void UpdateCameraVectors();
	private:
		glm::vec3 m_Position;
		glm::vec3 m_Front;
		glm::vec3 m_Up;
		glm::vec3 m_Right;
		glm::vec3 m_WorldUp;

		float m_Yaw;
		float m_Pitch;
		float m_Roll;

		// camera options
		float m_MovementSpeed;
		float m_MouseSensitivity;
		float m_FOV;

		float m_Near{0.1f};
		float m_Far{10.0f};

		static constexpr float DEFAULT_YAW = -90.0f;
		static constexpr float DEFAULT_PITCH = 0.0f;
		static constexpr float DEFAULT_SPEED = 2.5f;
		static constexpr float DEFAULT_SENSITIVITY = 0.1f;
		static constexpr float DEFAULT_FOV = 45.0f;
		static constexpr float DEFAULT_NEAR = 0.1f;
		static constexpr float DEFAULT_FAR = 10.0f;
	};
}