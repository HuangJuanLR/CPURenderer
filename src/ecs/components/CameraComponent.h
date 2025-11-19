#pragma once

namespace CPURDR
{
	struct CameraComponent
	{
		float fov = 60.0f;
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
		bool isActive = true;
		bool isPerspective = true;

		float orthoSize = 10.0f;

		CameraComponent() = default;
	};
}