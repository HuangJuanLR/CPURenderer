#include "MeshLoader.h"

#include "assimp/postprocess.h"
#include "plog/Log.h"

namespace CPURDR
{
	MeshLoader* MeshLoader::s_Instance = nullptr;

	MeshLoader& MeshLoader::GetInstance()
	{
		if (s_Instance == nullptr)
		{
			s_Instance = new MeshLoader();
		}
		return *s_Instance;
	}

	std::vector<Mesh> MeshLoader::LoadMeshFromFile(const std::string& filepath)
	{
		auto it = m_MeshCache.find(filepath);
		if (it != m_MeshCache.end())
		{
			PLOG_DEBUG << "Mesh loaded from cache: " << filepath;
			return it->second;
		}

		Assimp::Importer importer;
		const aiScene* pScene = importer.ReadFile(
			filepath.c_str(),
			aiProcess_Triangulate |
			aiProcess_GenNormals |
			aiProcess_JoinIdenticalVertices |
			aiProcess_OptimizeMeshes
		);

		if (pScene == nullptr)
		{
			PLOG_ERROR << "Failed to load mesh from file: " << filepath << " - " << importer.GetErrorString();
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", ("Error loading model: " + filepath).c_str(), nullptr);
			return std::vector<Mesh>();
		}

		if (pScene->mNumMeshes == 0)
		{
			PLOG_WARNING << "No meshes found in file: " << filepath;
			return std::vector<Mesh>();
		}

		std::vector<Mesh> meshes;
		meshes.reserve(pScene->mNumMeshes);

		for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
		{
			const aiMesh* aiMeshPtr = pScene->mMeshes[i];

			std::vector<Vertex> vertices;
			vertices.reserve(aiMeshPtr->mNumVertices);

			for (unsigned int v = 0; v < aiMeshPtr->mNumVertices; v++)
			{
				Vertex vertex;

				vertex.position = glm::vec3(
					aiMeshPtr->mVertices[v].x,
					aiMeshPtr->mVertices[v].y,
					aiMeshPtr->mVertices[v].z
				);

				if (aiMeshPtr->HasNormals())
				{
					vertex.normal = glm::vec3(
						aiMeshPtr->mNormals[v].x,
						aiMeshPtr->mNormals[v].y,
						aiMeshPtr->mNormals[v].z
					);
				}
				else
				{
					vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
				}

				if (aiMeshPtr->HasTextureCoords(0))
				{
					vertex.texcoord = glm::vec2(
						aiMeshPtr->mTextureCoords[0][v].x,
						aiMeshPtr->mTextureCoords[0][v].y
					);
				}
				else
				{
					vertex.texcoord = glm::vec2(0.0f, 1.0f);
				}

				vertices.push_back(vertex);
			}

			std::vector<unsigned int> indices;
			indices.reserve(aiMeshPtr->mNumFaces * 3);

			for (unsigned int f = 0; f < aiMeshPtr->mNumFaces; f++)
			{
				const aiFace& face = aiMeshPtr->mFaces[f];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
				{
					indices.push_back(face.mIndices[j]);
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

			meshes.emplace_back(vertices, indices, colors);

			PLOG_DEBUG << "Loaded submesh " << i << " from " << filepath;
		}

		m_MeshCache[filepath] = meshes;
		return meshes;
	}

	bool MeshLoader::IsMeshLoaded(const std::string& filepath) const
	{
		return m_MeshCache.find(filepath) != m_MeshCache.end();
	}

	const std::vector<Mesh>* MeshLoader::GetMesh(const std::string& filepath) const
	{
		auto it = m_MeshCache.find(filepath);
		return (it != m_MeshCache.end()) ? &it->second : nullptr;
	}

	void MeshLoader::ClearCache()
	{
		size_t count = m_MeshCache.size();
		m_MeshCache.clear();
		PLOG_DEBUG << "Cleared mesh cache (" << count << " meshes)";
	}

}
