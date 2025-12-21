#pragma once
#include "Model.h"

namespace CPURDR
{
	class Primitives
	{
	public:
		static Mesh Cube(float size = 1.0f);
		static Mesh Sphere(float radius = 0.5f, int segments = 32, int rings = 16);
		static Mesh Plane(float width = 1.0f, float height = 1.0f, int subdivisionX = 1, int subdivisionZ = 1);
		static Mesh Quad(float width = 1.0f, float height = 1.0f);
		static Mesh Cylinder(float radius = 0.5f, float height = 1.0f, int segments = 32);
		static Mesh Capsule(float radius = 0.5f, float height = 1.0f, int segments = 16, int rings = 8);

	private:
		// ==========================
		// For visualization only
		// ==========================
		static std::vector<SDL_Color> GenerateRandomColors(size_t count);

	private:
		static constexpr float PI = 3.14159265358979323846f;
	};
}
