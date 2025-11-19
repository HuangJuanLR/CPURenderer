#pragma once
#include <unordered_map>
#include <vector>

#include "Model.h"

namespace CPURDR
{
	class MeshLoader
	{
	public:
		static MeshLoader& GetInstance();

		std::vector<Mesh> LoadMeshFromFile(const std::string& filepath);

		bool IsMeshLoaded(const std::string& filepath) const;

		const std::vector<Mesh>* GetMesh(const std::string& filepath) const;

		void ClearCache();

	private:
		MeshLoader(): m_Rng(std::random_device()()), m_Dist(0, 255){};
		~MeshLoader() = default;

		MeshLoader(const MeshLoader&) = delete;
		MeshLoader& operator=(const MeshLoader&) = delete;
		MeshLoader(MeshLoader&&) = delete;
		MeshLoader& operator=(MeshLoader&&) = delete;

		std::unordered_map<std::string, std::vector<Mesh>> m_MeshCache;
		static  MeshLoader* s_Instance;

		// ==========================
		// For visualization only
		// ==========================
		std::mt19937 m_Rng;
		std::uniform_int_distribution<int> m_Dist;
	};
}
