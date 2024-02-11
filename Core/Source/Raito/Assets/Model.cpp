#include "pch.h"
#include "Model.h"

#include "Mesh.h"

namespace Raito::Assets
{
	Model::~Model()
	{
		for (auto mesh : m_Meshes)
		{
			delete mesh;
			mesh = nullptr;
		}

		m_Meshes.clear();
	}
}
