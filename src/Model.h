#pragma once
#include <string>
#include <vector>
#include <assimp/scene.h>

#include "vec2.hpp"
#include "vec3.hpp"
#include "assimp/Importer.hpp"
#include "SDL3/SDL.h"

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

	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
	: vertices(vertices), indices(indices)
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

	void DrawMesh();
private:


private:
	Assimp::Importer m_Importer;
	std::vector<Mesh> m_Meshes;
};
