#include "MeshFilter.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "plog/Log.h"

namespace CPURDR
{
	size_t MeshFilter::GetTotalVertexCount() const
	{
		size_t count = 0;
		for (const auto& mesh: meshes)
		{
			count += mesh.vertices.size();
		}
		return count;
	}

	size_t MeshFilter::GetTotalTriangleCount() const
	{
		size_t count = 0;
		for (const auto& mesh: meshes)
		{
			count += mesh.indices.size() / 3;
		}
		return count;
	}
}
