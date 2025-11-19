#pragma once
#include <vector>
#include "../../Model.h"

namespace CPURDR
{
	struct MeshFilter
	{
		std::vector<Mesh> meshes;

		MeshFilter() = default;
		explicit MeshFilter(const std::vector<Mesh>& m): meshes(m){}
		explicit MeshFilter(std::vector<Mesh>&& m): meshes(std::move(m)){}

		size_t GetTotalVertexCount() const;
		size_t GetTotalTriangleCount() const;
	};
}
