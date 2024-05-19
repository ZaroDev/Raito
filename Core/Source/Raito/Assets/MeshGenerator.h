#pragma once


#include "Mesh.h"

namespace Raito::Assets::MeshGenerator
{
	std::vector<Vertex> GenerateSphereVertices(float radius, u32 segments);
	std::vector<u32> GenerateSphereIndices(u32 segments);
	Mesh GenerateSphere(float radius, u32 segments);
}