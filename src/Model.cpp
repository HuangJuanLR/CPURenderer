#include "Model.h"

#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "SDL3/SDL_messagebox.h"

Model::Model(const std::string& file)
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
	}

	// const aiNode* node = pScene->mRootNode;
	// std::cout << pScene->mRootNode->mNumMeshes << std::endl;
	for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
	{
		const aiMesh* mesh = pScene->mMeshes[i];

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		glm::vec3 vector;

		for (unsigned int j = 0; j < mesh->mNumVertices; j++)
		{
			Vertex vertex;
			vector.x = mesh->mVertices[j].x;
			vector.y = mesh->mVertices[j].y;
			vector.z = mesh->mVertices[j].z;
			vertex.position = vector;

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
		m_Meshes.push_back(Mesh(vertices, indices));
	}

}

Model::~Model() noexcept
{
}
