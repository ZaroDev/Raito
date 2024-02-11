#include "pch.h"
#include "Mesh.h"

namespace Raito::Assets
{
	Mesh::Mesh(const std::vector<Vertex>& vertex, const std::vector<u32>& indices)
	: m_Vertices(vertex), m_Indices(indices)
	{
		
	}

	Mesh::~Mesh()
	{
		m_Vertices.clear();
		m_Indices.clear();
	}
}
