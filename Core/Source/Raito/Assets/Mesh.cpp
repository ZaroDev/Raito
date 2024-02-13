#include "pch.h"
#include "Mesh.h"


namespace Raito::Assets
{
	Mesh::Mesh(const std::vector<Vertex>& vertex, const std::vector<u32>& indices)
		: Vertices(vertex), Indices(indices)
	{

	}

	Mesh::~Mesh()
	{
		Vertices.clear();
		Indices.clear();
	}
}