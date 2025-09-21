#include "Model.h"

#include <iostream>

#include "App.h"
#include "Graphics.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "SDL3/SDL_messagebox.h"

Model::Model(const std::string& file):
	m_Rng(std::random_device{}()),
	m_Dist(0, 255)
{
	Assimp::Importer imp;

	const aiScene* pScene = imp.ReadFile(file.c_str(),
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenNormals);

	if (pScene == nullptr)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Error",
			"Error loading model",
			nullptr);

		return;
	}

	for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
	{
		const aiMesh* mesh = pScene->mMeshes[i];

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		glm::vec3 vector;

		glm::mat4 matRot = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, -1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
			);

		for (unsigned int j = 0; j < mesh->mNumVertices; j++)
		{
			Vertex vertex;
			vector.x = mesh->mVertices[j].x;
			vector.y = mesh->mVertices[j].y;
			vector.z = mesh->mVertices[j].z;
			glm::vec4 transformedPos = matRot * glm::vec4(vector, 1.0f);
			vertex.position = glm::vec3(transformedPos);

			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[j].x;
				vector.y = mesh->mNormals[j].y;
				vector.z = mesh->mNormals[j].z;
				vertex.normal = vector;
			}

			vertices.push_back(vertex);

		}

		for (unsigned int j = 0; j < mesh->mNumFaces; j++)
		{
			const aiFace face = mesh->mFaces[j];

			for (unsigned int k = 0; k < face.mNumIndices; k++)
			{
				indices.push_back(face.mIndices[k]);
			}
		}

		std::vector<SDL_Color> colors;
		colors.reserve(indices.size() / 3);
		for (unsigned int i = 0; i < indices.size(); i += 3)
		{
			SDL_Color c = {
				static_cast<uint8_t>(m_Dist(m_Rng)),
				static_cast<uint8_t>(m_Dist(m_Rng)),
				static_cast<uint8_t>(m_Dist(m_Rng)),
				255
			};
			colors.push_back(c);
		}

		m_Meshes.push_back(Mesh(vertices, indices, colors));
	}

}

Model::~Model() noexcept
{
}

void Model::DrawTriangle(SDL_Renderer* renderer)
{
	App& app = App::GetInstance();
	int width = app.GetWidth();
	int height = app.GetHeight();
	m_Zbuffer.reserve(width * height);

	for (int i = 0; i < m_Meshes.size(); i++)
	{
		auto vertices = m_Meshes[i].vertices;
		auto indices = m_Meshes[i].indices;
		for (int j = 0, t = 0; j < m_Meshes[i].indices.size(); j += 3, t++)
		{
			glm::vec3 p0 = vertices[indices[j]].position;
			glm::vec3 p1 = vertices[indices[j + 1]].position;
			glm::vec3 p2 = vertices[indices[j + 2]].position;
			p0 += glm::vec3(0, 0, 1);
			p0 *= glm::vec3(1, 1, 0.5);
			p1 += glm::vec3(0, 0, 1);
			p1 *= glm::vec3(1, 1, 0.5);
			p2 += glm::vec3(0, 0, 1);
			p2 *= glm::vec3(1, 1, 0.5);
			p0 = p0 * glm::vec3(300, 300, 255) + glm::vec3(480, 270, 0);
			p1 = p1 * glm::vec3(300, 300, 255) + glm::vec3(480, 270, 0);
			p2 = p2 * glm::vec3(300, 300, 255) + glm::vec3(480, 270, 0);
			const SDL_Color col = (t < m_Meshes[i].colors.size())? m_Meshes[i].colors[t] : SDL_Color{255, 255, 255, 255};
			SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
			Graphics::Triangle(renderer, p0, p1, p2, width, height, m_Zbuffer);
			// Graphics::Triangle(renderer, p0, p1, p2);
		}
	}
}

