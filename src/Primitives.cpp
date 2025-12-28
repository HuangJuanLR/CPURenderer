#include "Primitives.h"
#include <cmath>

#include "glm.hpp"

namespace CPURDR
{
	Mesh Primitives::Cube(float size)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		float half = size / 2.0f;

		// Front face(+Z)
		vertices.push_back({{-half, -half, half}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});
		vertices.push_back({{half, -half, half}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});
		vertices.push_back({{half, half, half}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});
		vertices.push_back({{-half, half, half}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});

		// Back face (-Z)
		vertices.push_back({{ half, -half, -half}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}});
		vertices.push_back({{-half, -half, -half}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}});
		vertices.push_back({{-half,  half, -half}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}});
		vertices.push_back({{ half,  half, -half}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}});

		// Right face (+X)
		vertices.push_back({{ half, -half,  half}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}});
		vertices.push_back({{ half, -half, -half}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}});
		vertices.push_back({{ half,  half, -half}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
		vertices.push_back({{ half,  half,  half}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});

		// Left face (-X)
		vertices.push_back({{-half, -half, -half}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}});
		vertices.push_back({{-half, -half,  half}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}});
		vertices.push_back({{-half,  half,  half}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}});
		vertices.push_back({{-half,  half, -half}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}});

		// Top face (+Y)
		vertices.push_back({{-half,  half,  half}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}});
		vertices.push_back({{ half,  half,  half}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}});
		vertices.push_back({{ half,  half, -half}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});
		vertices.push_back({{-half,  half, -half}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});

		// Bottom face (-Y)
		vertices.push_back({{-half, -half, -half}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}});
		vertices.push_back({{ half, -half, -half}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}});
		vertices.push_back({{ half, -half,  half}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}});
		vertices.push_back({{-half, -half,  half}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}});

		for (int face = 0; face < 6; face++)
		{
			int offset = face * 4;
			indices.push_back(offset + 0);
			indices.push_back(offset + 1);
			indices.push_back(offset + 2);
			indices.push_back(offset + 0);
			indices.push_back(offset + 2);
			indices.push_back(offset + 3);
		}

		auto colors = GenerateRandomColors(indices.size() / 3);
		return Mesh(vertices, indices, colors);
	}

	Mesh Primitives::Sphere(float radius, int segments, int rings)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		// Vertices
		for (int ring = 0; ring <= rings; ring++)
		{
			float phi = PI * (float)ring / (float)rings;
			float y = radius * std::cos(phi);
			float ringRadius = radius * std::sin(phi);
			for (int segment = 0; segment <= segments; segment++)
			{
				float theta = 2.0f * PI * (float)segment / (float)segments;
				float x = ringRadius * std::cos(theta);
				float z = ringRadius * std::sin(theta);

				glm::vec3 pos(x, y, z);
				glm::vec3 normal = glm::normalize(pos);
				glm::vec2 uv(
					(float)segment / (float)segments,
					(float)ring / (float)rings
					);

				vertices.push_back({pos, uv, normal});
			}
		}

		// Indices
		for (int ring = 0; ring < rings; ring++)
		{
			for (int segment = 0; segment < segments; segment++)
			{
				unsigned int current = ring * (segments + 1) + segment;
				unsigned int next = current + segments + 1;

				indices.push_back(current);
				indices.push_back(next);
				indices.push_back(current + 1);

				indices.push_back(current + 1);
				indices.push_back(next);
				indices.push_back(next + 1);
			}
		}

		auto colors = GenerateRandomColors(indices.size() / 3);
		return Mesh(vertices, indices, colors);
	}

	Mesh Primitives::Plane(float width, float height, int subdivisionX, int subdivisionZ)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		float halfWidth = width / 2.0f;
		float halfHeight = height / 2.0f;

		for (int z = 0; z <= subdivisionZ; z++)
		{
			for (int x = 0; x <= subdivisionX; x++)
			{
				float px = -halfWidth + (width * (float)x) / (float)subdivisionX;
				float pz = -halfHeight + (height * (float)z) / (float)subdivisionZ;

				glm::vec3 pos(px, 0.0f, pz);
				glm::vec2 uv(
					float(x) / float(subdivisionX),
					float(z) / float(subdivisionZ)
					);
				glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
				vertices.push_back({pos, uv, normal});
			}
		}

		for (int z = 0; z < subdivisionZ; z++)
		{
			for (int x = 0; x < subdivisionX; x++)
			{
				unsigned int tl = z * (subdivisionX + 1) + x;
				unsigned int tr = tl + 1;
				unsigned int bl = tl + (subdivisionX + 1);
				unsigned int br = bl + 1;

				indices.push_back(tl);
				indices.push_back(bl);
				indices.push_back(tr);

				indices.push_back(tr);
				indices.push_back(bl);
				indices.push_back(br);
			}
		}

		auto colors = GenerateRandomColors(indices.size() / 3);
		return Mesh{vertices, indices, colors};
	}

	Mesh Primitives::Quad(float width, float height)
	{
		return Plane(width, height, 1, 1);
	}

	Mesh Primitives::Cylinder(float radius, float height, int segments)
	{
		std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float halfHeight = height / 2.0f;

        // Top cap center
        vertices.push_back({{0.0f, halfHeight, 0.0f}, {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}});
        unsigned int topCenterIdx = 0;

        // Top cap rim
        for (int i = 0; i <= segments; i++)
        {
            float theta = 2.0f * PI * (float)i / (float)segments;
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);

            glm::vec2 uv(
                0.5f + 0.5f * std::cos(theta),
                0.5f + 0.5f * std::sin(theta)
            );

            vertices.push_back({{x, halfHeight, z}, uv, {0.0f, 1.0f, 0.0f}});
        }

        // Bottom cap center
        unsigned int bottomCenterIdx = (unsigned int)vertices.size();
        vertices.push_back({{0.0f, -halfHeight, 0.0f}, {0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}});

        // Bottom cap rim
        for (int i = 0; i <= segments; i++)
        {
            float theta = 2.0f * PI * (float)i / (float)segments;
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);

            glm::vec2 uv(
                0.5f + 0.5f * std::cos(theta),
                0.5f + 0.5f * std::sin(theta)
            );

            vertices.push_back({{x, -halfHeight, z}, uv, {0.0f, -1.0f, 0.0f}});
        }

        // Side vertices (need separate vertices for different normals)
        unsigned int sideStartIdx = (unsigned int)vertices.size();
        for (int i = 0; i <= segments; i++)
        {
            float theta = 2.0f * PI * (float)i / (float)segments;
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);

            glm::vec3 normal(std::cos(theta), 0.0f, std::sin(theta));
            float u = (float)i / (float)segments;

            // Top vertex
            vertices.push_back({{x, halfHeight, z}, {u, 0.0f}, normal});
            // Bottom vertex
            vertices.push_back({{x, -halfHeight, z}, {u, 1.0f}, normal});
        }

        // Top cap indices
        for (int i = 0; i < segments; i++)
        {
            indices.push_back(topCenterIdx);
            indices.push_back(topCenterIdx + i + 1);
            indices.push_back(topCenterIdx + i + 2);
        }

        // Bottom cap indices
        for (int i = 0; i < segments; i++)
        {
            indices.push_back(bottomCenterIdx);
            indices.push_back(bottomCenterIdx + i + 2);
            indices.push_back(bottomCenterIdx + i + 1);
        }

        // Side indices
        for (int i = 0; i < segments; i++)
        {
            unsigned int tl = sideStartIdx + i * 2;
            unsigned int bl = tl + 1;
            unsigned int tr = tl + 2;
            unsigned int br = tr + 1;

            indices.push_back(tl);
            indices.push_back(bl);
            indices.push_back(tr);

            indices.push_back(tr);
            indices.push_back(bl);
            indices.push_back(br);
        }

		auto colors = GenerateRandomColors(indices.size() / 3);
        return Mesh(vertices, indices, colors);
	}

	Mesh Primitives::Capsule(float radius, float height, int segments, int rings)
	{
		std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float cylinderHeight = height;
        float halfCylinderHeight = cylinderHeight / 2.0f;

        // Top hemisphere
        for (int ring = 0; ring <= rings; ring++)
        {
            float phi = (PI / 2.0f) * (float)ring / (float)rings;
            float y = radius * std::cos(phi) + halfCylinderHeight;
            float ringRadius = radius * std::sin(phi);

            for (int seg = 0; seg <= segments; seg++)
            {
                float theta = 2.0f * PI * (float)seg / (float)segments;
                float x = ringRadius * std::cos(theta);
                float z = ringRadius * std::sin(theta);

                glm::vec3 pos(x, y, z);
                glm::vec3 normal = glm::normalize(glm::vec3(x, y - halfCylinderHeight, z));
                glm::vec2 uv(
                    (float)seg / (float)segments,
                    (float)ring / (float)rings * 0.25f
                );

                vertices.push_back({pos, uv, normal});
            }
        }

        unsigned int topHemisphereVertCount = (rings + 1) * (segments + 1);

        // Bottom hemisphere
        for (int ring = 0; ring <= rings; ring++)
        {
            float phi = (PI / 2.0f) * (float)ring / (float)rings;
            float y = -radius * std::sin(phi) - halfCylinderHeight;
            float ringRadius = radius * std::cos(phi);

            for (int seg = 0; seg <= segments; seg++)
            {
                float theta = 2.0f * PI * (float)seg / (float)segments;
                float x = ringRadius * std::cos(theta);
                float z = ringRadius * std::sin(theta);

                glm::vec3 pos(x, y, z);
                glm::vec3 normal = glm::normalize(glm::vec3(x, y + halfCylinderHeight, z));
                glm::vec2 uv(
                    (float)seg / (float)segments,
                    0.75f + (float)ring / (float)rings * 0.25f
                );

                vertices.push_back({pos, uv, normal});
            }
        }

		unsigned int cylinderStartIdx = (unsigned int)vertices.size();
		for (int i = 0; i <= segments; i++)
		{
			float theta = 2.0f * PI * (float)i / (float)segments;
			float x = radius * std::cos(theta);
			float z = radius * std::sin(theta);

			glm::vec3 normal(std::cos(theta), 0.0f, std::sin(theta));
			float u = (float)i / (float)segments;

			// Top of cylinder (connects to bottom of top hemisphere)
			vertices.push_back({{x, halfCylinderHeight, z}, {u, 0.25f}, normal});
			// Bottom of cylinder (connects to top of bottom hemisphere)
			vertices.push_back({{x, -halfCylinderHeight, z}, {u, 0.75f}, normal});
		}

        // Top hemisphere indices
        for (int ring = 0; ring < rings; ring++)
        {
            for (int seg = 0; seg < segments; seg++)
            {
                unsigned int current = ring * (segments + 1) + seg;
                unsigned int next = current + segments + 1;

                indices.push_back(current);
                indices.push_back(next);
                indices.push_back(current + 1);

                indices.push_back(current + 1);
                indices.push_back(next);
                indices.push_back(next + 1);
            }
        }

        // Bottom hemisphere indices
        for (int ring = 0; ring < rings; ring++)
        {
            for (int seg = 0; seg < segments; seg++)
            {
                unsigned int current = topHemisphereVertCount + ring * (segments + 1) + seg;
                unsigned int next = current + segments + 1;

                indices.push_back(current);
                indices.push_back(next);
                indices.push_back(current + 1);

                indices.push_back(current + 1);
                indices.push_back(next);
                indices.push_back(next + 1);
            }
        }

		for (int i = 0; i < segments; i++)
		{
			unsigned int tl = cylinderStartIdx + i * 2;
			unsigned int bl = tl + 1;
			unsigned int tr = tl + 2;
			unsigned int br = tr + 1;

			indices.push_back(tl);
			indices.push_back(bl);
			indices.push_back(tr);

			indices.push_back(tr);
			indices.push_back(bl);
			indices.push_back(br);
		}

		auto colors = GenerateRandomColors(indices.size() / 3);
        return Mesh(vertices, indices, colors);
	}

	std::vector<SDL_Color> Primitives::GenerateRandomColors(size_t count)
	{
		static std::random_device rd;
		static std::mt19937 rng(rd());
		static std::uniform_int_distribution<int> dist(0, 255);

		std::vector<SDL_Color> colors;
		colors.reserve(count);

		for (size_t i = 0; i < count; i++)
		{
			colors.push_back({
				(uint8_t)dist(rng),
				(uint8_t)dist(rng),
				(uint8_t)dist(rng),
				255
			});
		}

		return colors;
	}
}
