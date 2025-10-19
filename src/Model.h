#pragma once
#include <random>
#include <string>
#include <vector>
#include <assimp/scene.h>
#include "vec2.hpp"
#include "vec3.hpp"
#include "assimp/Importer.hpp"
#include "SDL3/SDL.h"
#include "Texture2D.h"
#include "render/Context.h"

namespace CPURDR
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 texcoord;
		glm::vec3 normal;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<SDL_Color> colors;

		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
		: vertices(vertices), indices(indices)
		{

		}

		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<SDL_Color>& colors)
		: vertices(vertices), indices(indices), colors(colors)
		{

		}
	};

	class Model
	{
	public:
		Model(const std::string& file);
		~Model() noexcept;

		int GetVertices(const int index);
		int GetVertices(const int primitiveIndex, const int index);

		const std::vector<Mesh>& GetMeshes() const {return m_Meshes;}

		void Draw(Context* context);

		// Legacy
		void DrawTriangle(SDL_Renderer* renderer);
		const Texture2D_RGBA* GetColorBuffer() const {return &m_ColorBuffer;}
	private:
		Assimp::Importer m_Importer;
		std::vector<Mesh> m_Meshes;

		std::mt19937 m_Rng;
		std::uniform_int_distribution<int> m_Dist;

		// Legacy
		int m_ScreenWidth;
		int m_ScreenHeight;
		Texture2D_RFloat m_DepthBuffer;
		Texture2D_RGBA m_ColorBuffer;
	};
}
